#include <string>
#include <vector>

namespace tf{


    class Node{
    public:
        virtual std::string toString() = 0;
        virtual ~Node() {}
    };

    //-------------------------------------------------- Operator Types
    class BinaryOperator : public Node{
    public:
        virtual std::string toString() override = 0;
        BinaryOperator(Node* l , Node* r) : l(l) , r(r){}
        ~BinaryOperator() {delete l; delete r;};
    protected:
        Node* l;
        Node* r;
    };

    class UnaryOperator : public Node{
    public:
        virtual std::string toString() override = 0;
        UnaryOperator(Node* l ) : l(l) {}
        ~UnaryOperator() {delete l;};
    protected:
        Node* l;
    };
    //-------------------------------------------------- Functions
    class Function : public Node{
    public:
        Function(std::string& name , std::vector<Node*>* parms) : name(name) , parms(parms){}
        ~Function() {delete parms;};
        std::string toString() override;

    private:
        std::string name;
        std::vector<Node*>* parms;
    };
    //-------------------------------------------------- Constants
    class ConstNumber : public Node{
    public:
        ConstNumber(double l) : l(l){}
        std::string toString() override;
    private:
        double l;
    };

    class Variable : public Node{
    public:
        Variable(std::string l) : l(l){}
        std::string toString() override;
    private:
        std::string l;
    };
    //-------------------------------------------------- Binary Operators
    class Add : public BinaryOperator{
    public:
        Add(Node* l , Node* r) : BinaryOperator(l,r){}
        std::string toString() override;
    };

    class Sub : public BinaryOperator{
    public:
        Sub(Node* l , Node* r) : BinaryOperator(l,r){}
        std::string toString() override;
    };

    class Mul : public BinaryOperator{
    public:
        Mul(Node* l , Node* r) : BinaryOperator(l,r){}
        std::string toString() override;
    };

    class Pow : public BinaryOperator{
    public:
        Pow(Node* l , Node* r) : BinaryOperator(l,r){}
        std::string toString() override;
    };

    class Div : public BinaryOperator{
    public:
        Div(Node* l , Node* r) : BinaryOperator(l,r){}
        std::string toString() override;
    };
    //-------------------------------------------------- Unary Operators
    class Neg : public UnaryOperator{
    public:
        Neg(Node* l) : UnaryOperator(l){}
        std::string toString() override;
    };

    class Transpose : public UnaryOperator{
    public:
        Transpose(Node* l)  : UnaryOperator(l) {}
        std::string toString() override;
    };

    class Brace : public UnaryOperator{
    public:
        Brace(Node* l) : UnaryOperator(l){}
        std::string toString() override;
    };



}
