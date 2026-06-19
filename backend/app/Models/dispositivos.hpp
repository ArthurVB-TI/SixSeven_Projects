using namespace std;

class dispositivos{
    private:
        int id;
        string nome;
        string tipo;
        int id_usuario;

        void init_conections(){
            id = 0;
            nome = "";
            tipo = "";
            id_usuario = 0;
        }
        void destroy(){}
    public:
        dispositivos(){ init_conections(); }
        ~dispositivos(){ destroy(); }

        int get_id(){ return this->id; }
        string get_nome(){ return this->nome; }
        string get_tipo(){ return this->tipo; }
        int get_id_usuario(){ return this->id_usuario; }
        
        void set_id(int _id){ this->id = _id; }
        void set_nome(string _nome){ this->nome = _nome; }
        void set_id_usuario(int _id_usuario){ this->id_usuario = _id_usuario; }
};