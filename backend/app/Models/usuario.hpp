using namespace std;

class usuario{
    private:
        int id;
        string nome;
        string email;
        string senha;

        void init_usuario(){
            id = 0;
            nome = "";
            email = "";
            senha = "";
        }
        void destroy(){ }
    public:
        usuario(){ init_usuario(); }
        ~usuario(){ destroy(); }

        int get_id(){ return this->id; }
        string get_nome(){ return this->nome; }
        string get_email(){ return this->email; }
        string get_senha(){ return this->senha; }

        void set_id(int new_id){ this->id = new_id; }
        void set_nome(string new_nome){ this->nome = new_nome; }
        void set_email(string new_email){ this->email = new_email; }
        void set_senha(string new_senha){ this->senha = new_senha; }
};