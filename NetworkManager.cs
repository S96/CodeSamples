using System;
using System.Diagnostics;
using Lidgren.Network;

namespace OmegaRace
{
    class NetworkManager
    {
        // TYPES
        
        public enum INSTANCE_TYPE
        {
            SERVER_TYPE = 0,
            CLIENT_1_TYPE = 1,
            CLIENT_2_TYPE = 2
        }

        // PRIVATE MEMBERS
        
        private string _server_ip = "localhost";
        private int _server_port = 60616;
        private INSTANCE_TYPE _connection_type;
        private NetPeer _connection;

        // PROPERTIES

        public static INSTANCE_TYPE ConnectionType
        {
            get
            {
                return Instance()._connection_type;
            }
        }

        public static NetPeer Connection
        {
            get
            {
                return Instance()._connection;
            }
        }

        public static System.Net.IPEndPoint ServerEndPoint
        {
            get
            {
                return Instance().PrivGetServerEndPoint();
            }
        }

        public static string ServerIDString
        {
            get
            {
                return Instance().PrivGetServerIDString();
            }
        }

        // STATIC METHODS
        
        public static void ProcessData()
        {
            Instance().PrivProcessData();
        }

        public static INSTANCE_TYPE Initialize()
        {
            return Instance().PrivInitialize();
        }

        // PRIVATE METHODS

        private System.Net.IPEndPoint PrivGetServerEndPoint()
        {
            return NetUtility.Resolve(_server_ip, _server_port);
        }

        private string PrivGetServerIDString()
        {
            return _server_ip + ":" + _server_port;
        }

        private INSTANCE_TYPE PrivInitialize()
        {
            Console.WriteLine("\n****************\nEnter IP Address\n****************\n");
            Console.ForegroundColor = ConsoleColor.Yellow;
            _server_ip = Console.ReadLine();

            NetPeerConfiguration config = new NetPeerConfiguration("ServerClientClient");

            config.AcceptIncomingConnections = true;

            config.MaximumConnections = 100;

            config.MaximumHandshakeAttempts = 3;

            config.ResendHandshakeInterval = 0.5f;
            
            config.EnableMessageType(NetIncomingMessageType.DiscoveryResponse);

            _connection = new NetClient(config);
            Debug.Assert(_connection != null);

            _connection.Start();

            System.Net.IPEndPoint ep = NetUtility.Resolve(_server_ip, _server_port);

            NetIncomingMessage pInMsg;

            _connection.Connect(ep);
            
            bool connected = false;

            while (connected == false)
            {
                if ((pInMsg = _connection.ReadMessage()) != null)
                {
                    NetIncomingMessageType type = pInMsg.MessageType;
                    switch (type)
                    {
                        case NetIncomingMessageType.DebugMessage:
                            string debug1 = pInMsg.ReadString();
                            Debug.WriteLine(debug1);
                            break;

                        case NetIncomingMessageType.VerboseDebugMessage:
                            string debug2 = pInMsg.ReadString();
                            Debug.WriteLine(debug2);
                            break;

                        case NetIncomingMessageType.WarningMessage:
                            string warning = pInMsg.ReadString();
                            Debug.WriteLine(warning);
                            break;

                        case NetIncomingMessageType.ErrorMessage:
                            Debug.WriteLine(pInMsg.ReadString());
                            break;

                        case NetIncomingMessageType.StatusChanged:
                            NetConnectionStatus status = (NetConnectionStatus)pInMsg.ReadByte();
                            string reason = pInMsg.ReadString();
                            Debug.WriteLine(status.ToString() + ": " + reason);

                            if (status == NetConnectionStatus.Disconnected)
                            {
                                Debug.WriteLine("Server timeout. Becoming Server.");
                                _connection.Shutdown("");
                                InitializeServer();
                                connected = true;
                            }
                            else if (status == NetConnectionStatus.Connected)
                            {
                                Debug.WriteLine("Server found.");
                                InitializeClient();
                                connected = true;
                            }

                            break;

                        case NetIncomingMessageType.UnconnectedData:
                            Debug.WriteLine("Recv(" + pInMsg.SenderEndPoint + "): " + pInMsg.ReadString());
                            break;

                        case NetIncomingMessageType.DiscoveryResponse:
                            Debug.WriteLine("Found server at " + pInMsg.SenderEndPoint + " name: " + pInMsg.ReadString());
                            _connection.Connect(pInMsg.SenderEndPoint);
                            break;
                    }
                }
                
                _connection.Recycle(pInMsg);
            }

            return _connection_type;
        }

        private void PrivProcessData()
        {
            NetIncomingMessage buff;
            
            while ((_connection.ReadMessage(out buff)) == true)
            {
                NetIncomingMessageType type = buff.MessageType;
                switch (type)
                {
                    case NetIncomingMessageType.DebugMessage:
                        string debug1 = buff.ReadString();
                        Debug.WriteLine(debug1);
                        break;

                    case NetIncomingMessageType.VerboseDebugMessage:
                        string debug2 = buff.ReadString();
                        Debug.WriteLine(debug2);
                        break;

                    case NetIncomingMessageType.WarningMessage:
                        string warning = buff.ReadString();
                        Debug.WriteLine(warning);
                        break;

                    case NetIncomingMessageType.ErrorMessage:
                        Debug.WriteLine(buff.ReadString());
                        break;

                    case NetIncomingMessageType.StatusChanged:
                        NetConnectionStatus status = (NetConnectionStatus)buff.ReadByte();
                        string reason = buff.ReadString();
                        Debug.WriteLine(status.ToString() + ": " + reason);
                        break;

                    case NetIncomingMessageType.Data:

                        int t;

                        if (buff.ReadInt32(out t))
                        {
                            switch ((DataItem.DATA_TYPE)t)
                            {
                                case DataItem.DATA_TYPE.ACK_TYPE:

                                    break;

                                case DataItem.DATA_TYPE.EVENT_TYPE:
                                    EventData event_d = new EventData();
                                    event_d.Deserialize(ref buff);
                                    DataManager.PushToIn(new DataItem(DataItem.DATA_TYPE.EVENT_TYPE, event_d));
                                    break;

                                case DataItem.DATA_TYPE.INPUT_TYPE:
                                    InputData input = new InputData();
                                    input.Deserialize(ref buff);
                                    DataManager.PushToIn(new DataItem(DataItem.DATA_TYPE.INPUT_TYPE, input));
                                    break;

                                case DataItem.DATA_TYPE.SIM_DATA:
                                    SimData sim = new SimData();
                                    sim.Deserialize(ref buff);
                                    DataManager.PushToIn(new DataItem(DataItem.DATA_TYPE.SIM_DATA, sim));
                                    break;

                                case DataItem.DATA_TYPE.STATE_TYPE:
                                    StateData state = new StateData();
                                    state.Deserialize(ref buff);
                                    DataManager.PushToIn(new DataItem(DataItem.DATA_TYPE.STATE_TYPE, state));
                                    break;
                            }
                        }

                        break;

                    case NetIncomingMessageType.UnconnectedData:
                        Debug.WriteLine("Recv(" + buff.SenderEndPoint + "): " + buff.ReadString());
                        break;

                    case NetIncomingMessageType.DiscoveryResponse:
                        Debug.WriteLine("Found server at " + buff.SenderEndPoint + " name: " + buff.ReadString());
                        _connection.Connect(buff.SenderEndPoint);
                        break;
                }
                
                _connection.Recycle(buff);
            }
        }
        
        private void InitializeServer()
        {
            _connection_type = INSTANCE_TYPE.SERVER_TYPE;

            NetPeerConfiguration config = new NetPeerConfiguration("ServerClientClient");

            config.AcceptIncomingConnections = true;

            config.MaximumConnections = 2;
            
            config.Port = _server_port;
            
            config.EnableMessageType(NetIncomingMessageType.DiscoveryResponse);

            _connection = new NetServer(config);
            Debug.Assert(_connection != null);

            _connection.Start();

            NetIncomingMessage pInMsg;

            bool ready = false;

            Console.ForegroundColor = ConsoleColor.Gray;
            Console.WriteLine("\nServer Instance\nWaiting for clients...\n");

            int count = 0;

            while (!ready)
            {
                if ((pInMsg = _connection.ReadMessage()) != null &&
                        pInMsg.MessageType == NetIncomingMessageType.StatusChanged)
                {
                    if ((NetConnectionStatus)pInMsg.ReadByte() == NetConnectionStatus.Connected)
                    {
                        NetOutgoingMessage m = _connection.CreateMessage();
                        m.Write(_connection.ConnectionsCount);
                        _connection.SendMessage(m, _connection.Connections[_connection.ConnectionsCount - 1], NetDeliveryMethod.ReliableOrdered);

                        if(count != _connection.ConnectionsCount)
                        {
                            count = _connection.ConnectionsCount;
                            Console.WriteLine("\nClients connected: " + count + "\n");
                        }

                        if(_connection.ConnectionsCount >= 2)
                        {
                            ready = true;
                        }
                    }
                }
            }
        }

        private void InitializeClient()
        {
            bool ready = false;

            NetIncomingMessage pInMsg;

            while (!ready)
            {
                if((pInMsg = _connection.ReadMessage()) != null &&
                    pInMsg.MessageType == NetIncomingMessageType.Data)
                {
                    _connection_type = (INSTANCE_TYPE)pInMsg.ReadInt32();
                    Debug.Assert(_connection_type == INSTANCE_TYPE.CLIENT_1_TYPE || _connection_type == INSTANCE_TYPE.CLIENT_2_TYPE);
                    ready = true;
                }
            }
        }

        // SINGLETON MANAGEMENT

        private static NetworkManager Instance()
        {
            if(_instance == null)
            {
                _instance = new NetworkManager();
            }

            return _instance;
        }

        private static NetworkManager _instance;
    }
}