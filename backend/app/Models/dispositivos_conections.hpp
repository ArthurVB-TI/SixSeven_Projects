using namespace std;

class dispositivos_conections{
    private:
        int id_dispositivos;
        int id_conections;

        void init_conections(){
            id_dispositivos = 0;
            id_conections = 0;
        }
        void destroy(){}
    public:
        dispositivos_conections(){ init_conections(); }
        ~dispositivos_conections(){ destroy(); }

        int get_id_dispositivos(){ return this->id_dispositivos; }
        int get_id_conections(){ return this->id_conections; }

        void set_id_dispositivos(int id_dispositivos){ this->id_dispositivos = id_dispositivos; }
        void set_id_conections(int id_conections){ this->id_conections = id_conections; }
};