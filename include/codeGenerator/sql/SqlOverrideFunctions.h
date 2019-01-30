

Variable* generateCreateTensorFromDB(string newName, string oldName, vector<string>* l) override;
Variable* generateDefineTensor(string tenName, vector<Accessor*>* accs ,Variable* value) override;

Variable* generateSaveTensorToDB(string newName,string oldName, vector<string>* l) override;


Variable* generateIfElse(Variable* condition , string ifexps , string elseexps) override;
Variable* generateWhile(Variable* condition , string exps) override;
Variable* generateFor(string name,Variable* from , Variable* to , string exps) override;

Variable* generateFuncCall(string fname ,vector<Variable*>* parmlist ,vector<string>* returnList , vector<TypeDim>* retTypes) override;
Variable* generateDefineFunction(string fname ,vector<string>*  parmlist, vector<TypeDim>* parmTypes , vector<TypeDim>* retTypes ,string exps) override;
Variable* generateReturn(vector<Variable*>* returns) override;


string    generateBraceForList(string list) override;
Variable* generateTensorFromList(string list, size_t type, size_t oldDims) override;
Variable* generateAccsessTensor(Variable* v, vector<Accessor*>* ac) override;

Variable* generatePow(Variable* l , Variable* r) override;
Variable* generateTranspose(Variable* l) override;
Variable* generateMul(Variable* l , Variable* r) override;

Variable* generateBoolean(bool b) override;
Variable* generateXor (Variable* l , Variable* r) override;

Variable* generatePrint(vector<Variable*>* list) override;

string generateComparisonEquals() override;

Variable* generatePlot(string fileName ,Variable* xData ,Variable* yData  ,string xLable ,string yLable ,string type) override;
Variable* generateGradientDescent(string function , vector<string>* data ,  vector<pair<string,string>>* opt,
        Variable* stepSize,  Variable* steps, Variable* batchsize, Variable *threshold ) override;


Variable* generateSetVar(string name  , Variable*  r, bool remember) override;


stringstream declareVarsAndBegin() override;
stringstream getInit() override;
stringstream getStartMain() override;
stringstream getCleanup() override;

string    typeToString(TypeDim type) override;