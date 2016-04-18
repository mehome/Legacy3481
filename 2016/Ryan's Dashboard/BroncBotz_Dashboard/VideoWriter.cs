using Emgu.CV;
using System;
using System.Drawing;

namespace BroncBotz_Dashboard
{
    public class VideoWriterManager
    {
        private static Size size;

        private VideoWriterManager()
        {
        }

        private static VideoWriterManager instance;

        public bool Record { get; set; }

        private VideoWriter Writer = new VideoWriter(@"Dashboard_2016" + DateTime.Now.ToString("dd_MM_yyyy_HH_mm_ss") + ".avi", 24, size, true);

        public void WriteFrame(Mat frame)
        {
            size = new Size(frame.Width, frame.Height);
            Writer.Write(frame);
        }

        public static VideoWriterManager Instance
        {
            get
            {
                if (instance == null)
                {
                    instance = new VideoWriterManager();
                }
                return instance;
            }
        }
    }
}