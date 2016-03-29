//This is the link to what you see below
//  [11/23/2012 James]
// http://forums.iis.net/p/1176158/1994167.aspx

/*
Sign In
|Join

|

Blogs
Forums

Home›IIS.NET Forums›IIS 7 & IIS 8›Media›Encoding device with RTP protocol - we want to create a pull point in...


Encoding device with RTP protocol - we want to create a pull point in IIS 7.0 ( Live Smooth Streaming ) RSS


9 replies 
Last post Aug 31, 2011 05:42 AM by tomwg123
.
‹ Previous Thread|Next Thread ›
Reply

.

kroegerr
3 Posts

Encoding device with RTP protocol - we want to create a pull point in IIS 7.0 ( Live Smooth Strea...
																			   Mar 04, 2011 05:58 PM|LINK
																			   My company has purchased a VS-101-HDSDI (H.264 HD Video Server) from Marshal Electronics and we are trying to harness this device with Live Smooth Streaming and IIS 7.0.  This device uses the RTP session protocal and I have been trying to create a pull point to this device with Live Smooth streaming with no success.  I have found a different Media Service called Wowza Media Server 2 that seems to support RTP.   Is this the route we need to go or what am I missing?  I am a C# developer but usually work on the backend and middle-tier but have not worked with streaming video, etc, so any help is appreciated.
																			   Live Smooth StreamingH.264IIS 7 Media ServicesSilverlight Publishing Point PushRTP
																			   .
																			   Reply
																			   .
																			   samzhang
																			   248 Posts
																			   Microsoft


Re: Encoding device with RTP protocol - we want to create a pull point in IIS 7.0 ( Live Smooth S...
	Mar 04, 2011 07:50 PM|LINK
	IIS Live Smooth Streaming today does not support RTP stream ingest. 
	.
	Reply
	.
	kroegerr
	3 Posts


Re: Encoding device with RTP protocol - we want to create a pull point in IIS 7.0 ( Live Smooth S...
	Mar 04, 2011 10:13 PM|LINK

	Thank you for the reply.  We kind of thought that was the case but was hopeful. 
	.
	Reply

	.

	chriskno

	114 Posts


Re: Encoding device with RTP protocol - we want to create a pull point in IIS 7.0 ( Live Smooth S...
	Mar 05, 2011 02:17 AM|LINK
	Hi Randy,  Did you look at using Expression Encoder 4 Pro SP1?  With built-in multi-core support and CUDA-based GPU encoding, it's a match for many hardware encoders on the market.  See www.microsoft.com/expressionencoder to learn more...
	-Chris 


	encoder
	Chris Knowlton (MSFT) 
http://blogs.iis.net/chriskno 

Did my post answer your question? If so, please "Mark as Answer". 
This posting is provided "AS IS" with no warranties, and confers no rights. 
.
Reply

.

kroegerr

3 Posts


Re: Encoding device with RTP protocol - we want to create a pull point in IIS 7.0 ( Live Smooth S...
	Mar 07, 2011 03:52 PM|LINK
	Hi Chris,
	Thank you for your response.  However, our web server is a virtual machine so there is no way for us to attach a device directly to the server.  However, I will do some additional investigation to see if there is another way to add a live source to Encoder 4 that resides on a virutal machine.  Please let me know if you feel we can still do this as it will save me time in researching further.
	.
	Reply

	.

	cloudsurfin

	1 Post


Re: Encoding device with RTP protocol - we want to create a pull point in IIS 7.0 ( Live Smooth S...
	Apr 07, 2011 08:44 AM|LINK

	Chris, I hear this all the time from Microsoft trying to peddle its encoder software. The fact is, you can't put a computer and monitor all the places you can put a little hardware encoder. We're putting them on vehicles and airplanes for instance. How long untill we'll see IIS be capable of injesting generally standard RTP or RTSP streams? Wowza's not cutting it for us but it's the only option we've got. Thanks, Greg 
	.
	Reply
	.

	Nicolas.Drouin
	43 Posts


Re: Encoding device with RTP protocol - we want to create a pull point in IIS 7.0 ( Live Smooth S...
	Apr 19, 2011 07:04 PM|LINK




	We have completed a generic RTSP/RTP acquisition module for our Smooth Streaming Format SDK app.  We are able to injest RTSP/RTP streams from: Sony and Axis IP cameras and Antrica encoders.  Using the SSF SDK, we can POST to a live pub point or write .ism/.isma/.ismv files directly.

	You can find information on the smooth streaming format SDK here: 

http://www.iis.net/download/SmoothFormatSDK

-Nick
.
Reply

.

tomwg123

2 Posts


Re: Encoding device with RTP protocol - we want to create a pull point in IIS 7.0 ( Live Smooth S...
	Aug 25, 2011 09:18 PM|LINK
	That sounds just like what I want to do and am getting started with. I've found plenty of info related to the smooth streaming SDK and related protocol, but I've had difficulty finding information about doing the HTTP publishing. Do you or anyone else on here know any references that describe in more detail how the HTTP POST should go? I know it's supposed to be chunk encoded, but I'm not clear on what URL's to POST to. Also I'm unclear on the authentication process. I hoped to use wireshark with MS Expression Encoder to help understand better, but I don't have access to the full version which is required for live streaming. Thanks -Tom 
	.
	Reply
	.
	Nicolas.Drouin
	43 Posts


Re: Encoding device with RTP protocol - we want to create a pull point in IIS 7.0 ( Live Smooth S...
	Aug 29, 2011 03:56 PM|LINK
	For a windows applicaiton, I have used two approaches to publishing the chunks via HTTP:

- standard c++ using Winhttp.lib
- managed c# using HttpWebRequest from .Net
I used the native implementation when prototyping the technology.  When we went to production, we switched the publication of the chunks to managed code.  In both cases, we implement both HTTP and File modes of the SSF SDK, you will see some abstractions to support this in the code snippets below (I make no representations that these snippets are complete or suitable for anything but understanding the principles involved).
Also, make sure you understand what is explained here: http://blogs.iis.net/thalesc/archive/2010/03/15/iis-smooth-streaming-format-sdk-sample-application.aspx
And read the series of blogs by @SamZhang here: http://blogs.iis.net/samzhang/archive/2011/03/07/how-to-troubleshoot-live-smooth-streaming-issues-part-1.aspx
The native approach uses Winhttp.lib, in particular: 
*/

HRESULT SSFHTTPWriter::beginWriting(const wchar_t* i_szStreamName)
{
	//Return value:
	HRESULT hr = E_FAIL;


	// Use WinHttpOpen to obtain a session handle.
	m_hSession = WinHttpOpen( m_szClientName, WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );

	// Specify an HTTP server.
	if( m_hSession )
	{
		//logMessage("SSFHTTPWriter: Session Opened.\n");
		m_hConnect = WinHttpConnect( m_hSession, m_szServerName, _wtoi(m_szServerPort), 0 );
	}

	// Open a request.
	if( m_hConnect )
	{
		//logMessage("SSFHTTPWriter: Session Connected.\n");
		wchar_t szBuf[MAX_PATH];
		if (_snwprintf_s(szBuf,MAX_PATH,L"%s/Streams(%s)",m_szPublishingPoint,m_szStreamName) != -1)
		{
			m_hRequest = WinHttpOpenRequest( m_hConnect, L"POST", szBuf, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0 );
		}
	}

	if( m_hRequest )
	{
		//logMessage("SSFHTTPWriter: Request Opened.\n");
		hr = S_OK;
	}

	return hr;
}

/*
You asked about which URL to POST to: you can see the format in "%s/Streams(%s)" in WinHttpOpenRequest. 
That is, you have to connect to something that looks like:
http://MyServer:MyPort/MyPubpointPathInclucingSubFolders/Stream(MyUniqueStreamBitrateName)
Where MyUniqueSteamBitrateName matches the SSF_STREAM_INFO.pszStreamName passed to the SSF SDK during the AddStreamToMux call for this stream.
You need to open a connection and make a request for EACH stream that you want to post.  So if you have two video bitrates and one audio stream, that's three WinHttpOpenRequest calls, each with a different (unique) stream name.
Thereafter, the main application will make calls to these:
*/

HRESULT SSFHTTPWriter::writeHeader(LPCWSTR pszFileMode, LPCVOID pbData, ULONG cbData)
{
	HRESULT hr = E_FAIL;
	hr = writeDataToServer(pbData, cbData);
	return hr;
}

HRESULT SSFHTTPWriter::writeSampleData(LPCWSTR pszFileMode, LPCVOID pbData, ULONG cbData)
{
	HRESULT hr = E_FAIL;
	hr = writeDataToServer(pbData, cbData);
	return hr;
}

HRESULT SSFHTTPWriter::writeIndex(LPCWSTR pszFileMode, LPCVOID pbData, ULONG cbData)
{
	HRESULT hr = E_FAIL;
	hr = writeDataToServer(pbData, cbData);
	return hr;
}

Where:


HRESULT SSFHTTPWriter::writeDataToServer(LPCVOID pbData, ULONG cbData)
{
	//The calls to a server are asynch, we need to cache the data and use worker threads.

	HRESULT hr = E_FAIL;
	if (cbData>0  && m_hRequest)
	{
		//Create a local copy of the data
		BYTE *pbDataThread = NULL;
		pbDataThread = new BYTE[cbData];
		memcpy(pbDataThread,pbData,cbData);

		//Create a thread

		//Create the data-packet for the thread, including:
		// m_hRequest, pbDataThread, and the value of cbData.

		//Call the following in that thread:
		{
			// Send a Request:
			BOOL bResults=FALSE;
			if (m_hRequest) 
				bResults = WinHttpSendRequest( m_hRequest, 
				WINHTTP_NO_ADDITIONAL_HEADERS, NULL, 
				pbDataThread, cbData, 
				cbData, 0);

			// Wait for the server to treat the responce:
			if (bResults)
			{
				if (m_iLogCount < 10)
				{
					//logMessage("SSFHTTPWriter: Data Posted to Server.\n");
					m_iLogCount++;
				}
				bResults = WinHttpReceiveResponse(m_hRequest, NULL);
			}

			if (bResults)
				hr = S_OK;
			if (m_iLogCount < 11)
			{
				//logMessage("SSFHTTPWriter: Received Response from Server.\n");
			}    
			//Clean up memory
			//This should be done by the thread
			SAFE_DELETE(pbDataThread);
		}
	}
	return hr;
}

//Note: that there is no threading or chunk queue management in this prototype code; you should add it!
//		  Remember to clean up the connections :

	  HRESULT SSFHTTPWriter::endWriting()
	  {

		  //Clean up threads

		  // Close any open handles.
		  if (m_hRequest) WinHttpCloseHandle(m_hRequest);
		  if (m_hConnect) WinHttpCloseHandle(m_hConnect);
		  if (m_hSession) WinHttpCloseHandle(m_hSession);

		  return S_OK;
	  }


//	  Note that to close the streams and shutdown the chunked encoding correctly, your main application should be doing some logic that looks like this:

	  // Get the stream indexes via SSFMuxGetIndex
	  //In bLive mode, posting this sends an empty mfra box and shuts down the stream.
	  //Otherwise, the mfra box is appended to the .ismv/isma files.
	  hr = GetStreamIndexes(&hSSFMux, iStreamCount);
	  if( FAILED(hr) ) 
	  { 
		  printf( "Error 0x%08x: Failed to GetStreamHeaders\n", hr ); 
	  }
	  //Write Indexes:
	  hr = WriteIndexBuffers(iStreamCount, L"a+b");
	  if( FAILED(hr) ) 
	  { 
		  printf( "Error 0x%08x: Failed to WriteIndexBuffers\n", hr ); 
	  }

	  //Close HTTP Chunked-Encoding:
	  for(int i=0; i<iStreamCount; i++)
	  {
		  char zero='0';
		  g_StreamData[i].pWriter->PostChunk(&zero,0); //Where this eventually calls writeSampleData from above
	  } 

//	  The managed approach is a bit cleaner, but you need to pass your byte buffer up from the native C++ code, up to managed C++, through to C#.  This might seem trivial, but you will find that working with mixed-mode projects in VS2010 is not very pleasant... editorializing aside, here's how the C# bit goes.

//		  In C#, we've used the following:


		  public override void Initialize()
	  {

		  ...

			  //This must match the wSsfStreamInfo.pszSourceFileName
			  AudioStreamUri = new Uri(BasePublishingLocation + "/" + BaseName + ".isml/Streams(" + BaseName + "_audio)");

		  //Check web permission
		  WebPermission wp = new WebPermission(PermissionState.Unrestricted);
		  wp.AddPermission(NetworkAccess.Connect, AudioStreamUri.ToString());
		  wp.Demand(); //Throws a security exception if denied


		  if (HasAudio)
		  {
			  AudioWebRequest = (HttpWebRequest) WebRequest.Create(AudioStreamUri);
			  AudioWebRequest.AllowWriteStreamBuffering = false;
			  AudioWebRequest.Method = "POST";
			  AudioWebRequest.SendChunked = true;
			  AudioWebRequest.ContentLength = 0;
			  AudioWebRequest.KeepAlive = true;
			  AudioWebRequest.Timeout = -1; // Infinite  // +++++ Watch out with this, if you don't have a server at the other end, you will get in trouble!!!
			  AudioWebRequest.ReadWriteTimeout = -1; // Infinite
			  AudioStream = AudioWebRequest.GetRequestStream(); // This can hang the app.
		  }

		  VideoStreamUri = new Uri(BasePublishingLocation + "/" + BaseName + ".isml/Streams(" + BaseName + ")");

		  VideoWebRequest = (HttpWebRequest)WebRequest.Create(VideoStreamUri);
		  VideoWebRequest.AllowWriteStreamBuffering = false;
		  VideoWebRequest.Method = "POST";
		  VideoWebRequest.SendChunked = true;
		  VideoWebRequest.ContentLength = 0;
		  VideoWebRequest.KeepAlive = true;
		  VideoWebRequest.Timeout = -1; // Infinite
		  VideoWebRequest.ReadWriteTimeout = -1; // Infinite
		  VideoStream = VideoWebRequest.GetRequestStream(); // This can hang the app.

	  }

	  //Thereafter, as above, we use:


	  private Boolean WriteAudioHeaderOnce = true;

	  public override void WriteAudioHeader(byte[] buffer)
	  {
		  if (!HasAudio)
			  return;

		  // We do not need to re-write the updated headers in live mode.
		  if (WriteAudioHeaderOnce == true)
		  {
			  PostAudioToServer(buffer);
			  WriteAudioHeaderOnce = false;
		  }
	  }

	  public override void WriteAudioIndex(byte[] buffer)
	  {
		  if (!HasAudio)
			  return;
		  PostAudioToServer(buffer);
	  }

	  public override void WriteAudioSample(byte[] buffer)
	  {
		  if (!HasAudio)
			  return;
		  PostAudioToServer(buffer);
	  }

	  public override void WriteClientManifest(byte[] buffer)
	  {
		  // Manifests are generated by the IIS Media Server.
		  // Do not create/post manifests in HTTP mode.
		  return;
	  }

	  public override void WriteServerManifest(byte[] buffer)
	  {
		  // Manifests are generated by the IIS Media Server.
		  // Do not create/post manifests in HTTP mode.
		  return;
	  }

//	  Note that we implement an interface here, so the Manifest calls are discarded because we are in HTTP mode.  There are similar funcitons for the Video streams.

//		  All of the above call:

	  private void PostAudioToServer(byte[] buffer)
	  {
		  if (!HasAudio)
			  return;

		  if (AudioStream != null && AudioWebRequest != null)
		  {
			  try
			  {
				  AudioStream.Write(buffer, 0, buffer.Length);
			  }
			  catch (IOException ioException)
			  {
				  Shutdown();
				  Container.Resolve<IExceptionManager>().Publish(new Exception(String.Format("AudioStream.Write failed for {0}", BaseName), ioException));
			  }
			  catch (NotSupportedException ioConcurrentRead)
			  {
				  Shutdown();
				  Container.Resolve<IExceptionManager>().Publish(new Exception(String.Format("AudioStream.Write failed for {0}.  This is normal while debugging AudioVideo.Server.", BaseName), ioConcurrentRead));
			  }
		  }
	  }

//	  To shutdown, you do need to send the index (empty mfra box). However, in contrast the the WinHttp.lib example, there is no need in C# to send a closing frame (the character "0") via chunked encoding.  Simply close the connection and .Net will send the closing frame for you :

	  public override void Shutdown()
	  {
		  if (VideoStream != null)
		  {
			  VideoStream.Close();
		  }

		  VideoWebRequest = null;
		  WriteVideoHeaderOnce = true;

		  if (AudioStream != null)
		  {
			  AudioStream.Close();
		  }

		  AudioWebRequest = null;
		  WriteAudioHeaderOnce = true;

	  }
/*
NOTE:  .Net implements the standards for HTTP1.1 where a typical web client application will default to a maximum number of concurrent connections to one server.  This will be troublesome if you want to publish many streams at one time.  You need to set:
	  <system.net>
		  <connectionManagement>
		  <add address="*" maxconnection="1000"/>
		  </connectionManagement>
		  </system.net>
		  Otherwise your app will hang when trying to open new connections.

		  Regards,

		  -Nick

		  .
		  Reply

		  .

		  tomwg123

		  2 Posts


Re: Encoding device with RTP protocol - we want to create a pull point in IIS 7.0 ( Live Smooth S...

	Aug 31, 2011 05:42 AM|LINK

	Wow, thank you very much for taking the time to write that detailed reply.  Should be very helpful for me. 
	.


	‹ Previous Thread|Next Thread ›
	.

	This site is hosted for Microsoft by Neudesic, LLC. | © 2012 Microsoft. All rights reserved.
	Privacy Statement
	|Terms of Use
	|Contact Us
	|Advertise With Us
	Follow us on:
	  Twitter
		  |Facebook
		  Feedback on IIS
		  |Powered by IIS8


		  .




		  .
		  */