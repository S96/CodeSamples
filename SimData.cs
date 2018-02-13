namespace OmegaRace
{
    public class SimData
    {
        public enum SIM_OBJ
        {
            SHIP_OBJ,
            MISSILIE_OBJ,
            MINE_OBJ,
            FENCE_OBJ
        }

        public enum OBJ_ACTION
        {
            NEW_OBJ,
            UPDATE_OBJ,
            DESTROY_OBJ
        }

        public SimData()
        {

        }

        public SimData(PLAYER p, SIM_OBJ o, OBJ_ACTION a, Box2DX.Common.Vec2 pos, float angle, int id)
        {
            _player = p;
            _obj = o;
            _action = a;
            _pos = pos;
            _angle = angle;
            _inst_id = id;
        }
        
        public PLAYER Player
        {
            get
            {
                return _player;
            }
        }

        public SIM_OBJ SimObject
        {
            get
            {
                return _obj;
            }
        }

        public OBJ_ACTION Action
        {
            get
            {
                return _action;
            }
        }

        public Box2DX.Common.Vec2 Pos
        {
            get
            {
                return _pos;
            }
        }

        public float Angle
        {
            get
            {
                return _angle;
            }
        }

        public int InstID
        {
            get
            {
                return _inst_id;
            }
            set
            {
                _inst_id = value;
            }
        }

        public void Serialize(ref Lidgren.Network.NetOutgoingMessage buff)
        {
            buff.Write((int)DataItem.DATA_TYPE.SIM_DATA);
            buff.Write((int)_player);
            buff.Write((int)_obj);
            buff.Write((int)_action);
            buff.Write(_pos[0]);
            buff.Write(_pos[1]);
            buff.Write(_angle);
            buff.Write(_inst_id);
        }

        public void Deserialize(ref Lidgren.Network.NetIncomingMessage buff)
        {
            _player = (PLAYER)buff.ReadInt32();
            _obj = (SIM_OBJ)buff.ReadInt32();
            _action = (OBJ_ACTION)buff.ReadInt32();
            _pos[0] = buff.ReadFloat();
            _pos[1] = buff.ReadFloat();
            _angle = buff.ReadFloat();
            _inst_id = buff.ReadInt32();
        }

        private Box2DX.Common.Vec2 _pos;
        private PLAYER _player;
        private SIM_OBJ _obj;
        private OBJ_ACTION _action;
        private float _angle;
        private int _inst_id;
    }
}
