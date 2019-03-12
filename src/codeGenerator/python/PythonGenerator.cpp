#include "codeGenerator/python/PythonGenerator.h"

namespace CodeGenerator {

	Variable *PythonGenerator::generateCreateTensorFromDB(string newName, string oldName, vector<string> *l) {
		stringstream ss;
		ss << oldName << "[[";
		string sep;
		for (const string &s : *l) {
			ss << sep << '\'' << s << '\'';
            sep = ", ";
		}
		ss << "]].values";
		return generateSetVar(newName, new Variable(ss.str(), Variable::FLOAT, 2), true);
	}

	Variable *PythonGenerator::generateDefineTensor(string tenName, vector<Accessor *> *accs, Variable *value) {
		stringstream ss;
		ss << "np.full( (";
		char sep = ' ';
		for (Accessor *acc : *accs) {
			if (!acc->isSingle()) {
				throw ERROR("Can not define Tensors with : operator");
			}
			ss << sep << acc->getFrom()->getContext();
			sep = ',';
		}
		ss << "), " << value->getContext() << ')';
		TypeDim td(value->getType(), accs->size());
		Variable *r = new Variable(ss.str(), td);
		return generateSetVar(tenName, r, true);
	}

	Variable *PythonGenerator::generateReadCsv(string newName, string filename, vector<string> *cols, string del,
											   bool deleteEmpty,
											   Variable *replaceEmptys, vector<pair<string, string>> *replaceList,
											   bool createCsv, bool useModified) {
		usePandas = true;
		stringstream ss;
		ss << newName << " = pd.read_csv('" << filename;
		if(useModified){
			ss << ".ml.csv";
		}
		ss << "', delimiter=" << del << ", usecols=[";
		string sep;
		string names;
		for (string const &s : *cols) {
			ss << sep << "'" << s << "'";
			names += sep;
			names += s;
			sep = ",";
		}
		ss << "] )" << endl;
		if (deleteEmpty) {
			ss << newName << ".dropna(inplace=True)";
		} else {
			ss << newName << ".fillna(" << replaceEmptys << ", inplace=True)";
		}

		if(createCsv){
			createvsv(filename , names , del , replaceList);
		}


		setVarToType(newName, TypeDim(Variable::FLOAT, 2));
		return new Variable(ss.str(), Variable::VOID);
	}


	Variable *PythonGenerator::generateSaveTensorToDB(string newName, string oldName, vector<string> *l) {
		return new Variable("#Python can not save to DB " + oldName, Variable::NONE);
	}

	Variable *PythonGenerator::generateSaveTensorToCSV(string newName, string filename, vector<string> *l, string del) {
		stringstream ss;
		ss << "pd.DataFrame( data=" << newName << " , columns=[";
		string sep;
		for (string const &s : *l) {
			ss << sep << "'" << s << "'";
			sep = ",";
		}
		ss << "]).to_csv('" << filename << "', sep='" << del << "' ,  index=False )";
		return new Variable(ss.str(), Variable::VOID);
	}


	Variable *PythonGenerator::generateIfElse(Variable *condition, string ifexps, string elseexps) {
		stringstream ss;
		ss << "if " << condition->getContext() << ":" << endl;
		ss << ifexps;
		if (!elseexps.empty()) {
			ss << endl << "else: " << endl;
			ss << elseexps;
		}
		return new Variable(ss.str(), Variable::VOID);
	}

	Variable *PythonGenerator::generateWhile(Variable *condition, string exps) {
		stringstream ss;
		ss << "while " << condition->getContext() << ":" << endl;
		ss << exps;
		return new Variable(ss.str(), Variable::VOID);
	}

	Variable *PythonGenerator::generateFor(string name, Variable *from, Variable *to, string exps) {
		stringstream ss;
		ss << "for " << name << " in range(" << from->getContext() << "," << to->getContext() << "):" << endl;
		ss << exps;
		return new Variable(ss.str(), Variable::VOID);
	}


	Variable *PythonGenerator::generateFuncCall(string fname, vector<Variable *> *parmlist, vector<string> *returnList,
												vector<TypeDim> *retTypes) {
		stringstream ss;
		string sep;
		if (returnList->size() > retTypes->size()) {
			throw ERROR("To many arguments expected as return");
		}
		if (!returnList->empty()) {
			for (size_t i = 0; i < returnList->size(); i++) {
				string s = (*returnList)[i];
				TypeDim type = (*retTypes)[i];
				setVarToType(s, type);
				ss << sep << s;
				sep = ",";
			}
			ss << " = ";
		}
		ss << fname << "(";
		sep = "";
		for (Variable *v : *parmlist) {
			ss << sep << v->getContext();
			sep = ",";
		}
		ss << ")";
		return new Variable(ss.str(), Variable::VOID);
	}

	Variable *PythonGenerator::generateDefineFunction(string fname, vector<string> *parmlist, vector<TypeDim> *,
													  vector<TypeDim> *, string exps) {
		stringstream ss;
		string sep;
		ss << "def " << fname << "(";
		for (string const &s : *parmlist) {
			ss << sep << s;
			sep = ",";
		}
		ss << "):" << endl;
		ss << exps;
		return new Variable(ss.str(), Variable::VOID);
	}

	Variable *PythonGenerator::generateReturn(vector<Variable *> *returns) {
		stringstream ss;
		if (!returns->empty()) {
			ss << "return ";
			string sep;
			for (Variable *v : *returns) {
				ss << sep << v->getContext();
				sep = ",";
			}
		}
		return new Variable(ss.str(), Variable::VOID);
	}


	Variable *PythonGenerator::generateTensorFromList(string list, size_t type, size_t oldDims) {
        if(oldDims == -1){
            oldDims++;
        }
		return new Variable("np.array([" + list + "])", type, oldDims + 1);
	}

	Variable *PythonGenerator::generateAppend(Variable *l, Variable *r) {
		stringstream res;
		res << "np.append(" << l->getContext() << "," << r->getContext() << ", axis=0)";
		return new Variable(res.str(), l->getTypeDim());
	}

	Variable *PythonGenerator::generateAccsessTensor(Variable *v, vector<Accessor *> *acc) {
		TypeDim td = v->getTypeDim();
		size_t dim = td.second;
		stringstream res;

		//We need this beacause Python does not slice as the ml language does
		bool slicing = false;
		if (v->getDim() == acc->size()) {
			for (Accessor *ac : *acc) {
				slicing = slicing || !ac->isSingle();
			}
		} else {
			slicing = true;
		}

		res << v->getContext() << '[';
		char sep = ' ';
		for (Accessor *ac : *acc) {
			res << sep;
			if (ac->isSingle()) {
				res << ac->getFrom()->getContext();
				if(slicing){
					res << ":" << ac->getFrom()->getContext() << " +1";
				}
				if (ac->getFrom()->getDim() != 0) {
					throw ERROR("You can not access a tensor with a tensor");
				}
				dim--;
			} else if (ac->isFromTo()) {
				res << ac->getFrom()->getContext() << ":" << ac->getTo()->getContext() << " +1";
				if (ac->getFrom()->getDim() != 0 || ac->getTo()->getDim() != 0) {
					throw ERROR("You can not access a tensor with a tensor");
				}
			} else {
				res << ':';
			}
			sep = ',';
		}
		res << ']';

        if(slicing) {
            return new Variable(res.str(), td);
        }if (dim !=  0) {
			//Sice SQL allows no dim reduction!
			string begin = "np.array(";
			string end;
			for (size_t i = dim; i < td.second; i++) {
				begin += '[';
				end += ']';
			}
			end += ')';
			stringstream ss;
			ss << begin << res.rdbuf() << end;
			return new Variable(ss.str(), td);
		} else {
			return new Variable(res.str(), td.first, 0);
		}


	}

	string PythonGenerator::generateBraceForList(string list) {
		string res = "[";
		res += list;
		res += "]";
		return res;
	}


	Variable *PythonGenerator::generateSetVar(string name, Variable *r, bool remember) {
		if (remember) {
			setVarToType(name, r->getTypeDim());
		}
		stringstream ss;
		ss << name << " = " << r->getContext();;
		return new Variable(ss.str(), Variable::VOID);
	}

	Variable* PythonGenerator::generateNum(string num , int type){
		return new Variable(num , type );
	}

	Variable *PythonGenerator::generateMul(Variable *l, Variable *r) {
		if(l->getDim() != r->getDim() && l->getDim() != 0 && r->getDim() != 0  ){
			throw ERROR("Dimensions of tensor(dot) product does not match!");
		}
		if (l->getDim() > 2 ){
			stringstream ss;
			ss << "np.tensordot( " << l->getContext() << "," << r->getContext() << ",axes=0)";
			size_t dim = max(r->getDim(), l->getDim());
			return new Variable(ss.str(), Variable::FLOAT, dim);
		} else if (l->getDim() == 2 || l->getDim() == 1 ){
			stringstream ss;
			ss << "np.dot( " << l->getContext() << "," << r->getContext() << ")";
			size_t dim = max(r->getDim(), l->getDim());
			return new Variable(ss.str(), Variable::FLOAT, dim);
		} else {
			return mathHelper(l, r, "*");
		}
	}

	Variable *PythonGenerator::generatePow(Variable *l, Variable *r) {
		if(r->getDim() != 0){
			throw ERROR("The dimension of the rigth side of pow is not zero");
		}
		stringstream ss;
		switch (l->getDim()) {
			case 0:
				ss << l->getContext() << "** (" << r->getContext() << ")";
				break;
			case 2:
				ss << "np.linalg.matrix_power(" << l->getContext() << ", " << r->getContext() << ")";
				break;
			default:
				throw ERROR("Power can only be used on matrices and numbers");
		}
		return new Variable(ss.str(), l->getTypeDim());
	}

	Variable *PythonGenerator::generateTranspose(Variable *l) {
		return new Variable(l->getContext() + ".T", l->getTypeDim());
	}

	Variable *PythonGenerator::generateRandom() {
		return new Variable("np.random.random_sample()", Variable::FLOAT, 0);
	}

	Variable *PythonGenerator::generateBoolean(bool b) {
		return new Variable(b ? "True" : "False", Variable::BOOL);
	}

	Variable *PythonGenerator::generateXor(Variable *l, Variable *r) {
		return mathHelper(l, r, "^");
	}

	string PythonGenerator::generateComparisonEquals() {
		return string("==");
	}


	Variable *PythonGenerator::generatePrint(vector<Variable *> *list) {
		stringstream ss;
		ss << "print( ";
		char sep = ' ';
		if (!list->empty()) {
			string format =(*list)[0]->getContext();
			format = std::regex_replace(format , std::regex("%") , "{}");
			ss << format;
			ss << ".format( ";
		}
		for (size_t i = 1; i < list->size(); i++) {
			Variable *v = (*list)[i];
			ss << sep << v->getContext();
			sep = ',';
		}
		ss << "))";
		return new Variable(ss.str(), Variable::VOID);
	}


	stringstream PythonGenerator::declareVarsAndBegin() {
		stringstream ss;
		ss << "\n";
		return ss;
	}

	stringstream PythonGenerator::getInit() {
		stringstream ss;
		ss << "import numpy as np" << endl;
		if (usePandas) {
			ss << "import pandas as pd" << endl;
		}
		if (useTensorFlow) {
			ss << "import tensorflow as tf" << endl;
		}
		if (useMatPlotLib) {
			ss << "import matplotlib" << endl
			   << "import matplotlib.pyplot as plt" << endl;
		}

		return ss;
	}

	stringstream PythonGenerator::getStartMain() {
		stringstream ss;
		ss << "def main():\n";
		return ss;
	}

	stringstream PythonGenerator::getCleanup() {
		stringstream ss;
		ss << "main()";
		return ss;
	}

	Variable *
	PythonGenerator::generatePlot(string fileName, Variable *xData, Variable *yData, string xLable, string yLable,
								  string type) {
		useMatPlotLib = true;
		stringstream ss;
		ss << "plt.plot(" << xData->getContext() << "," << yData->getContext() << "," << type << ")" << endl;
		ss << "plt.xlabel(" << xLable << ")" << endl;
		ss << "plt.ylabel(" << yLable << ")" << endl;
		ss << "plt.savefig(" << fileName << ")" << endl;
		return new Variable(ss.str(), Variable::VOID);
	}

//https://medium.com/coinmonks/stochastic-vs-mini-batch-training-in-machine-learning-using-tensorflow-and-python-7f9709143ee2


    Variable *PythonGenerator::generateGradientDescent(string function, vector<string> *data,  vector<pair<string,string>>* opt,
            Variable *stepSize, Variable *maxSteps, Variable *batchsize, Variable *threshold) {

		tf::TFGraphBuilder lossGraph(function);
		if (lossGraph.hasError()) {
			throw ERROR("Function could not be parsed!");
		}

		//We need Tensorflow to do this:
		useTensorFlow = true;
		stringstream ss;

		string batchsizes = batchsize ? batchsize->getContext(): GD_BATCH_SIZE_DEFAULT;
        string maxStepss = maxSteps ? maxSteps->getContext(): GD_MAX_STPES_DEFAULT;
        string stepSizes = stepSize ? stepSize->getContext(): GD_LEARNING_RATE_DEFAULT;
        string thresholds = threshold ? threshold->getContext(): GD_THRESHOLD_DEFAULT;


		//Create session

		ss << endl << "#Gradient descent:" << endl;
		ss << "ML_sess = tf.Session()" << endl;

		//Generate Placeholders
		ss << "#Placeholders: " << endl;
		for (string &s : *data) {
			//Tensorflow will find out shape by its own!
			ss << "ML_tf_" << s << " = " << "tf.placeholder(shape=None , dtype=tf.float64)" << endl;
		}

		//Generate Variables
		ss << "#Variables: " << endl;
		for(pair<string,string> &p : *opt){
		    string& s = p.first;
            string& shape = p.second;
			//Tensorflow will find out shape by its own!
			if(nameToType.find(s) != nameToType.end()){
				//Variable already defined so take it as start value!
				ss << "ML_tf_" << s << " = " << "tf.Variable(tf.constant(" << s << ", dtype=tf.float64))" << endl;
			}else{
				//Variable not defined so take 0 vector it as start value!
				ss << "ML_tf_" << s << " = " << "tf.Variable(tf.constant(0. , shape=" << shape << ", dtype=tf.float64))" << endl;
			}

		}


		ss << "#Lossfunction:" << endl;
		ss << "ML_loss =  tf.reduce_mean(" << lossGraph.toTFGraph() << ")" << endl;


		ss << "ML_opt = tf.train.GradientDescentOptimizer(learning_rate=" << stepSizes << ")" << endl;
		ss << "ML_train_step = ML_opt.minimize(ML_loss)" << endl;

		ss << "#Initialize variables:" << endl;
		ss << "ML_init = tf.global_variables_initializer()" << endl;
		ss << "ML_sess.run(ML_init)" << endl;

		string feed_dict = "feed_dict={";
		//Feed the dict
		string sep;
		for (string &s : *data) {
			feed_dict += ML_TAB;
			feed_dict += sep;
			feed_dict += "ML_tf_";
			feed_dict += s;
			feed_dict += ": ML_batch_";
			feed_dict += s;
			sep = ", ";
		}
		feed_dict += "}";

		ss << "for i in range(" << maxStepss << "):" << endl;

		ss << ML_TAB << "ML_batch_indices = np.random.choice(np.arange(len( " << (*data)[0] << " )), " << batchsizes
				<< ", replace = False)" << endl; // Create Batch indecies
		for (string &s : *data) {
			ss << ML_TAB << "ML_batch_" << s << " = " << s << "[ML_batch_indices]" << endl;
		}

		//Run the training step
		ss << ML_TAB << "ML_sess.run(ML_train_step," << feed_dict << " )" << endl;


		/* Add printing every--- steps
         * if(i+1) % 5 == 0:
            print('Step #', str(i+1), 'W = ', str(ML_sess.run(ML_tf_w)))
            temp_loss = ML_sess.run(ML_loss, feed_dict={ML_tf_X: ML_batch_X, ML_tf_y: ML_batch_y})
            print('Loss = ', temp_loss)
         */

		//Now set the results!
		for (pair<string,string> &p : *opt) {
			ss << p.first << " = ML_sess.run(ML_tf_" << p.first << ")" << endl;
		}


		return new Variable(ss.str(), Variable::VOID);
	}

};

