class Dados{
    private:
        void init_dados(){
            this->id = 0;
            this->E_b = 0;
            this->E_r = 0;
            this->timer = 0;
            this->M_er = 0;
            this->V_mer = 0;
            this->M_vmer = 0;
        }
    protected:
        int id;
        int E_b;
        int E_r;
        int timer;
        int M_er;
        int V_mer;
        int M_vmer;
    public:
        Dados(){ init_dados(); }

        int get_E_b(){ return this->E_b; }
        int get_E_r(){ return this->E_r; }
        int get_timer(){ return this->timer; }
        int get_M_er(){ return this->M_er; }
        int get_V_mer(){ return this->V_mer; }
        int get_M_vmer(){ return this->M_vmer; }

        void set_E_b(int E_b){ this->E_b = E_b; }
        void set_E_r(int E_r){ this->E_r = E_r; }
        void set_timer(int timer){ this->timer = timer; }
        void set_M_er(int M_er){ this->M_er = M_er; }
        void set_V_mer(int V_mer){ this->V_mer = V_mer; }
        void set_M_vmer(int M_vmer){ this->M_vmer = M_vmer; }
};

class Dados_Repository : public Dados{
    public:
        // Metodos
        void Create(int id,int E_b,int E_r,int timer,int M_er,int V_mer,int M_vmer){
            this->id = id;
            this->E_b = E_b;
            this->E_r = E_r;
            this->timer = timer;
            this->M_er = M_er;
            this->V_mer = V_mer;
            this->M_vmer = M_vmer;
        }

        void Update(int id,int E_b,int E_r,int timer,int M_er,int V_mer,int M_vmer){
            this->id = id;
            this->E_b = E_b;
            this->E_r = E_r;
            this->timer = timer;
            this->M_er = M_er;
            this->V_mer = V_mer;
            this->M_vmer = M_vmer;
        }
};