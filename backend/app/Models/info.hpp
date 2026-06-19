using namespace std;

class info{
    private:
        int id;
        int E_b;
        int E_r;
        int timer;
        int M_er;
        int V_mer;
        int M_vmer;

        void init_info(){
            id = 0;
            E_b = 0;
            E_r = 0;
            timer = 0;
            M_er = 0;
            V_mer = 0;
            M_vmer = 0;
        }
        void destroy(){}
    public:
        info(){ init_info(); }
        ~info(){ destroy(); }

        int get_id(){ return this->id; }
        int get_E_b(){ return this->E_b; }
        int get_E_r(){ return this->E_r; }
        int get_timer(){ return this->timer; }
        int get_M_er(){ return this->M_er; }
        int get_V_mer(){ return this->V_mer; }
        int get_M_vmer(){ return this->M_vmer; }

        void set_id(int _id){ this->id = _id; }
        void set_E_b(int _E_b){ this->E_b = _E_b; }
        void set_E_r(int _E_r){ this->E_r = _E_r; }
        void set_timer(int _timer){ this->timer = _timer; }
        void set_M_er(int _M_er){ this->M_er = _M_er; }
        void set_V_mer(int _V_mer){ this->V_mer = _V_mer; }
        void set_M_vmer(int _M_vmer){ this->M_vmer = _M_vmer; }
};
