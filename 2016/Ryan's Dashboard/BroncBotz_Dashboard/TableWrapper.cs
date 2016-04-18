using NetworkTables;

namespace BroncBotz_Dashboard
{
    public class TableWrapper
    {
        public NetworkTable Table { get; set; }

        private TableWrapper()
        {
        }

        private static TableWrapper instance;

        public static TableWrapper Instance
        {
            get
            {
                if (instance == null)
                {
                    instance = new TableWrapper();
                }
                return instance;
            }
        }
    }
}