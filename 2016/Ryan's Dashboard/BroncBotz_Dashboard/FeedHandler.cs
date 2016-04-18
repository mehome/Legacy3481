using BroncBotz_Dashboard.Properties;
using Emgu.CV;
using Emgu.CV.CvEnum;
using Emgu.CV.Structure;
using Emgu.CV.UI;
using Emgu.CV.Util;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.ExceptionServices;
using System.Security;
using System.Threading;

namespace BroncBotz_Dashboard
{
    public class FeedHandler : IDisposable
    {
        private Mat temp;
        private Capture capture;
        private static Mat logo;
        private MainWindow parent;
        private BackgroundWorker bw;
        private bool terminate = false;
        private ImageBox destOutputImage, destCompositOutputImage;

        /// <summary>
        /// boolean to enable and disable targeting
        /// </summary>
        public bool Targeting { get; set; }

        #region Constructors
        public FeedHandler(Capture input, ImageBox normal, ImageBox composit, MainWindow main)
        {
            parent = main;
            capture = input;
            Targeting = true;
            destOutputImage = normal;
            destCompositOutputImage = composit;

            logo = CvInvoke.Imread(@"defaultFeed.jpg", Emgu.CV.CvEnum.LoadImageType.Color);

            bw = new BackgroundWorker();
            bw.WorkerSupportsCancellation = true;
            bw.DoWork += Bw_DoWork;
            bw.RunWorkerAsync();
        }

        public FeedHandler(ImageBox normal, ImageBox composit)
        {
            destOutputImage = normal;
            destCompositOutputImage = composit;

            logo = CvInvoke.Imread(@"defaultFeed.jpg", Emgu.CV.CvEnum.LoadImageType.Color);

            destOutputImage.Image = logo;
            destCompositOutputImage.Image = logo;

            destOutputImage.Invalidate();
            destCompositOutputImage.Invalidate();
        }
        #endregion

        private void Bw_DoWork(object sender, DoWorkEventArgs e)
        {
            while (!terminate)
            {
                update(null, new EventArgs());
                Thread.Sleep(25);
            }
        }


        [HandleProcessCorruptedStateExceptions]
        [SecurityCritical]
        private void update(object sender, EventArgs e)
        {
            try
            {
                temp = capture.QueryFrame();

                if (Targeting && temp != null)
                {
                    var output = processImage(temp);
                    destOutputImage.Invoke(new Action(() => destOutputImage.Image = output.Item1));
                    destOutputImage.Invoke(new Action(() => destCompositOutputImage.Image = output.Item2));

                    if (VideoWriterManager.Instance.Record)
                        VideoWriterManager.Instance.WriteFrame(temp);
                }
                else if (temp != null)
                {
                    destOutputImage.Invoke(new Action(() => destOutputImage.Image = temp));
                    destOutputImage.Invoke(new Action(() => destCompositOutputImage.Image = logo));

                    if (VideoWriterManager.Instance.Record)
                        VideoWriterManager.Instance.WriteFrame(temp);
                }
                else
                {
                    destOutputImage.Invoke(new Action(() => destOutputImage.Image = logo));
                    destOutputImage.Invoke(new Action(() => destCompositOutputImage.Image = logo));
                }
            }
            catch (Exception) { if (temp != null) temp.Dispose(); }
        }

        //This is where frames are proccessed
        private Tuple<Mat, Image<Gray, byte>> processImage(Mat original)
        {
            if (original != null)
            {
                Image<Hsv, byte> hsv_image = original.ToImage<Hsv, byte>();
                Hsv lowerLimit = new Hsv(Settings.Default.lowerHue, Settings.Default.lowerSaturation, Settings.Default.lowerBrightness);
                Hsv upperLimit = new Hsv(Settings.Default.upperHue, Settings.Default.upperSaturation, Settings.Default.upperBrightness);
                var imageHSVDest = hsv_image.InRange(lowerLimit, upperLimit);
                var imageHSVDest_ = hsv_image.InRange(lowerLimit, upperLimit);

                List<RotatedRect> boxList = new List<RotatedRect>();
                CircleF circle = new CircleF();
                List<CircleF> circles = new List<CircleF>();

                int contourSize = 0;

                //find contours and draw smallest possible circle around it
                using (VectorOfVectorOfPoint contours = new VectorOfVectorOfPoint())
                {
                    CvInvoke.FindContours(imageHSVDest_, contours, null, RetrType.List, ChainApproxMethod.ChainApproxSimple);
                    contourSize = contours.Size;
                    if (contourSize > 0)
                        for (int i = 0; i < contourSize; i++)
                        {
                            circle = CvInvoke.MinEnclosingCircle(contours[i]);
                            circles.Add(circle);
                        }
                }

                MCvScalar red = new MCvScalar(0, 0, 255);
                CircleF largestAndClosest = new CircleF();//where our final circle will be stored
                List<CircleF> temp = new List<CircleF>(circles);//list of circles to weed out

                if (temp.Count != 0)
                {
                    List<CircleF> sortedCircles = temp.OrderBy(o => o.Area).ToList();
                    sortedCircles.Reverse();//reverse to largets to smallest

                    foreach (CircleF c in sortedCircles)
                        if (!(c.Area > 2500 && c.Area < 25000))
                            temp.Remove(c);//remove any circle that does not fit our area criteria

                    if (temp.Count == 0)
                        return new Tuple<Mat, Image<Gray, byte>>(original, imageHSVDest);//if there are no good targets, return the frame unaltered

                    sortedCircles = temp.OrderBy(o => o.Area).ToList();
                    sortedCircles.Reverse();

                    int xCentre = original.Size.Width / 2;
                    int yCentre = original.Size.Height / 2;
                    largestAndClosest = sortedCircles[0];
                    if (sortedCircles.Count != 1)
                    {
                        CircleF secondHit = sortedCircles[1];
                        float offset = (secondHit.Center.X - xCentre);
                        float offset2 = (largestAndClosest.Center.X - xCentre);
                        float offsetY = (secondHit.Center.Y - yCentre);
                        float offsetY2 = (largestAndClosest.Center.Y - yCentre);

                        //find the largest circle that is closest to the centre of the screen
                        if (offset < offset2 || (offsetY < offsetY2 && !(offset > offset2)))
                            largestAndClosest = secondHit;
                    }

                    //update information, should there be a valid target
                    if (contourSize != 0)
                    {
                        CvInvoke.Circle(original, new System.Drawing.Point((int)largestAndClosest.Center.X, (int)largestAndClosest.Center.Y), (int)largestAndClosest.Radius, red, 2, LineType.AntiAlias);

                        if (((int)largestAndClosest.Center.X - xCentre) >= -10 && ((int)largestAndClosest.Center.X - xCentre) <= 10)
                            parent.UpdateOffsetLabel(((int)largestAndClosest.Center.X - xCentre), System.Windows.Media.Brushes.LimeGreen);
                        else
                            parent.UpdateOffsetLabel(((int)largestAndClosest.Center.X - xCentre), System.Windows.Media.Brushes.Red);

                        if (((int)largestAndClosest.Radius) >= 41 && ((int)largestAndClosest.Radius) <= 45)
                            parent.UpdateRaduisLabel(((int)largestAndClosest.Radius), System.Windows.Media.Brushes.LimeGreen);
                        else
                            parent.UpdateRaduisLabel(((int)largestAndClosest.Radius), System.Windows.Media.Brushes.Red);

                        TableWrapper.Instance.Table.PutNumber("VISION_OFFSET", ((int)largestAndClosest.Center.X - xCentre));
                        TableWrapper.Instance.Table.PutNumber("CIRCLE_RADIUS", (int)largestAndClosest.Radius);
                    }
                    else
                    {
                        parent.UpdateOffsetLabel(0, null, true);
                    }

                    return new Tuple<Mat, Image<Gray, byte>>(original, imageHSVDest);
                }
                else
                    return new Tuple<Mat, Image<Gray, byte>>(original, imageHSVDest);
            }
            return new Tuple<Mat, Image<Gray, byte>>(null, null);
        }

        //dispose of unmanaged resources
        [HandleProcessCorruptedStateExceptions]
        [SecurityCritical]
        public void Dispose()
        {
            if (capture != null)
            {
                try
                {
                    capture.Stop();
                    capture.Dispose();
                }
                catch (Exception) { }
            }
            if (bw != null)
            {
                terminate = true;
                bw.Dispose();
            }
        }
    }
}