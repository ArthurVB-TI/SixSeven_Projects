using namespace std;

class conections{
    private:
        int id;
        string nome;
        bool canActive;
        bool isActive;
        int id_usuario;

        void init_conections(){
            id = 0;
            nome = "";
            canActive = false;
            isActive = false;
        }
        void destroy(){}
    public:
        conections(){ init_conections(); }
        ~conections(){ destroy(); }

        int get_id(){ return this->id; }
        string get_nome(){ return this->nome; }
        bool get_canActive(){ return this->canActive; }
        bool get_isActive(){ return this->isActive; }
        int get_id_usuario(){ return this->id_usuario; }
        
        void set_id(int _id){ this->id = _id; }
        void set_nome(string _nome){ this->nome = _nome; }
        void set_canActive(bool _canActive){ this->canActive = _canActive; }
        void set_isActive(bool _isActive){ this->isActive = _isActive; }
        void set_id_usuario(int _id_usuario){ this->id_usuario = _id_usuario; }
};