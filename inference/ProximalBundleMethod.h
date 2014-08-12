#ifndef HOST_INFERENCE_PROXIMAL_BUNDLE_METHOD_H__
#define HOST_INFERENCE_PROXIMAL_BUNDLE_METHOD_H__

#include <gurobi_c++.h>
#include <util/exceptions.h>
#include <util/Logger.h>

extern logger::LogChannel proxbundlemethodlog;

class BundleCannotSolveQpException : public Exception {};

template <typename ValueGradientCallback>
class ProximalBundleMethod {

	typedef std::vector<std::vector<double> > rows_type;

public:

	/**
	 * The status of the bundle method.
	 */
	enum Status {

		NotStarted,

		ExactOptimiumFound,

		Converged,

		IterationsExceeded,

		Error
	};

	ProximalBundleMethod(
		unsigned int numDims,
		unsigned int numIterations,
		ValueGradientCallback& valueGradientCallback,
		double eps = 1e-5,
		double rho = 1e-3,
		double regularizerWeight = 1.0);

	~ProximalBundleMethod();

	template <typename IteratorType>
	void setInitialPosition(IteratorType positionBegin, IteratorType positionEnd);

	/**
	 * Add a hyperplane to the initial bundle (optional). A hyperplane is 
	 * described by a gradient a and the function value at 0, b.
	 *
	 * @param aBegin, aEnd
	 *              Begin and end iterators for the values of a.
	 *
	 * @param b
	 *              The value of the hyperplane at 0.
	 */
	template <typename IteratorType>
	void addInitialHyperplane(IteratorType aBegin, IteratorType aEnd, double b);

	bool optimize();

	const std::vector<double>& getOptimalPosition() const;

	double getOptimalValue() const;

	Status getStatus() const { return _status; }

private:

	/**
	 * Called whenever proxCenter changed (i.e., after a serious step).
	 */
	void setObjective();

	void updateProxCenter(const std::vector<double>& position, double value);

	bool addHyperplane(const std::vector<double>& position, double value, const std::vector<double>& gradient_tp1);

	void solveQP(std::vector<double>& position, double& value, double& qpValue);

	unsigned int countNonzero(const std::vector<double>& values);

	double dot(const std::vector<double>& a, const std::vector<double>& b);

	void getModelLog();

	void log();

	unsigned int _numDims;
	unsigned int _numIterations;
	unsigned int _iteration;

	// the bundle (gradients in A, offsets in b)
	rows_type           _A;
	std::vector<double> _b;

	ValueGradientCallback& _valueGradientCallback;

	double _eps;
	double _rho;
	double _regularizerWeight;
	double _steplength;

	std::vector<double> _initialPosition;
	std::vector<double> _proxCenter_t;
	std::vector<double> _previous_proxCenter_t;
	std::vector<double> _previous_gradient;
	std::vector<double> _lambda_tp1;
	std::vector<double> _lambda_t;
	std::vector<double> _gradient_tp1;

	double _value_tp1;
	double _bundleValue_tp1;
	double _qpValue_tp1;
	double _eps_t;
	double _proxValue_t;
	double _improvement;

	std::vector<double> _optimalPosition;
	double              _optimalValue;
	double              _optimalEps;

	GRBEnv      _env;
	GRBModel    _model;
	GRBVar*     _variables;
	GRBVar      _slack;
	GRBQuadExpr _objective;

	Status _status;
};

/*
 * IMPLEMENTATION
 */

template <typename ValueGradientCallback>
ProximalBundleMethod<ValueGradientCallback>::ProximalBundleMethod(
	unsigned int numDims,
	unsigned int numIterations,
	ValueGradientCallback& valueGradientCallback,
	double eps,
	double rho,
	double regularizerWeight) :
_numDims(numDims),
_numIterations(numIterations),
_valueGradientCallback(valueGradientCallback),
_eps(eps),
_rho(rho),
_regularizerWeight(regularizerWeight),
_steplength(0),
_initialPosition(numDims),
_proxCenter_t(numDims),
_previous_proxCenter_t(numDims),
_lambda_tp1(numDims),
_lambda_t(numDims),
_gradient_tp1(numDims),
_optimalPosition(_initialPosition),
_optimalValue(0),
_optimalEps(-1) /* not started, yet */,
_model(_env),
_status(NotStarted) {

	_model.getEnv().set(GRB_IntParam_OutputFlag, 0);

	LOG_DEBUG(proxbundlemethodlog) << "adding variables" << std::endl;

	// add one variable for each dimension
	_variables = _model.addVars(_numDims, GRB_CONTINUOUS);

	// add the slack variable
	_slack = _model.addVar(-GRB_INFINITY, GRB_INFINITY, 0, GRB_CONTINUOUS);

	_model.update();
}

template <typename ValueGradientCallback>
ProximalBundleMethod<ValueGradientCallback>::~ProximalBundleMethod() {

	if (_variables)
		delete[] _variables;
}

template <typename ValueGradientCallback>
template <typename IteratorType>
void
ProximalBundleMethod<ValueGradientCallback>::setInitialPosition(IteratorType positionBegin, IteratorType positionEnd) {

	std::copy(positionBegin(), positionEnd(), _initialPosition.begin());
	std::copy(positionBegin(), positionEnd(), _proxCenter_t.begin());
}

/**
 * Add a hyperplane to the initial bundle (optional). A hyperplane is 
 * described by a gradient a and the function value at 0, b.
 *
 * @param aBegin, aEnd
 *              Begin and end iterators for the values of a.
 *
 * @param b
 *              The value of the hyperplane at 0.
 */
template <typename ValueGradientCallback>
template <typename IteratorType>
void
ProximalBundleMethod<ValueGradientCallback>::addInitialHyperplane(IteratorType aBegin, IteratorType aEnd, double b) {

	if (aEnd - aBegin != _numDims)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"gradient of initial hyperplane has wrong dimensions: "
				<< (aEnd - aBegin) << ", should be "
				<< _numDims << std::endl);

	_A.push_back(std::vector<double>(aEnd - aBegin));
	std::copy(aBegin, aEnd, _A.rbegin()->begin());

	_b.push_back(b);
}

template <typename ValueGradientCallback>
bool
ProximalBundleMethod<ValueGradientCallback>::optimize() {

	_previous_gradient.clear();

	_lambda_tp1            = _initialPosition;
	_previous_proxCenter_t = _initialPosition;

	LOG_DEBUG(proxbundlemethodlog) << "adding " << _A.size() << " initial constraints" << std::endl;

	// add the initial constraints Ax - b ≤ ξ

	rows_type::const_iterator           row;
	std::vector<double>::const_iterator b;
	for (row = _A.begin(), b = _b.begin(); row != _A.end(); row++, b++) {

		GRBLinExpr expr;
		expr.addTerms(&(*row)[0], _variables, row->size());
		expr -= _slack;

		_model.addConstr(expr, GRB_GREATER_EQUAL, -(*b));
	}

	_model.update();

	LOG_DEBUG(proxbundlemethodlog) << "computing first value and gradient" << std::endl;

	// compute value ξ' and gradient g' of original objective
	_valueGradientCallback(_initialPosition, _value_tp1, _gradient_tp1);

	// set prox center and initialize objective
	updateProxCenter(_initialPosition, _value_tp1);

	// set initial hyperplane
	addHyperplane(_initialPosition, _value_tp1, _gradient_tp1);

	for (_iteration = 0; _iteration < _numIterations; _iteration++) {

		LOG_DEBUG(proxbundlemethodlog) << "iteration #" << _iteration << std::endl;

		try {

			_lambda_t = _lambda_tp1;

			solveQP(_lambda_tp1, _bundleValue_tp1, _qpValue_tp1);

		} catch (BundleCannotSolveQpException e) {

			LOG_DEBUG(proxbundlemethodlog) << "couldn't solve the QP" << std::endl;
			_status = Error;

			return false;
		}

		LOG_DEBUG(proxbundlemethodlog) << "computing value and gradient at current λ^(t+1)" << std::endl;

		// L(λ^(t+1)) (and the gradient, to be used in the next iteration)
		_valueGradientCallback(_lambda_tp1, _value_tp1, _gradient_tp1);

		// terminated?
		_eps_t = _qpValue_tp1 - _proxValue_t;

		// serious step?
		_improvement = _value_tp1 - _proxValue_t;

		// note: we need to log before we update the model again
		getModelLog();

		// we found a horizontal plane -- we can stop searching further
		if (countNonzero(_gradient_tp1) == 0) {

			LOG_DEBUG(proxbundlemethodlog) << "Encountered zero gradient -- exact optimum found!" << std::endl;
			LOG_DEBUG(proxbundlemethodlog) << "\tℒ(λ^(t+1)) = " << _bundleValue_tp1 << std::endl;
			LOG_DEBUG(proxbundlemethodlog) << "\tL(λ^(t+1)) = " << _value_tp1 << std::endl;

			_optimalPosition = _lambda_tp1;
			_optimalValue    = _value_tp1;
			_optimalEps      = 0; // hurray!
			_status          = ExactOptimiumFound;

			log();

			return true;
		}

		if (_eps_t < _eps) {

			LOG_DEBUG(proxbundlemethodlog) << "converged to desired precision (" << _eps_t << " < " << _eps << ")" << std::endl;
			LOG_DEBUG(proxbundlemethodlog) << "\tℒ(λ^(t+1)) = " << _bundleValue_tp1 << std::endl;
			LOG_DEBUG(proxbundlemethodlog) << "\tL(λ^(t+1)) = " << _value_tp1 << std::endl;
			LOG_DEBUG(proxbundlemethodlog) << "\teps_t      = " << _eps_t << std::endl;

			_optimalPosition = _lambda_tp1;
			_optimalValue    = _value_tp1;
			_optimalEps      = _eps_t;
			_status          = Converged;

			log();

			return true;
		}

		// try to add new hyperplane
		bool added = addHyperplane(_lambda_tp1, _value_tp1, _gradient_tp1);

		// if the improvement is better than a multiple of the eps_t
		// OR
		// the current hyperplane is already part of the bundle we change the prox
		// center to guarantee termination
		if (!added || _improvement > _rho*_eps_t) {

			if (_improvement < 0) {

				LOG_DEBUG(proxbundlemethodlog) << "################ WARNING #####################" << std::endl;
				LOG_DEBUG(proxbundlemethodlog) << "perform serious step with negative improvement" << std::endl;
				LOG_DEBUG(proxbundlemethodlog) << "##############################################" << std::endl;
			}

			LOG_DEBUG(proxbundlemethodlog) << "serious step performed -- update prox center" << std::endl;

			updateProxCenter(_lambda_tp1, _value_tp1);
		}

		log();

		//LOG_DEBUG(proxbundlemethodlog) << "current position is " << _lambda_tp1 << std::endl;
		//LOG_DEBUG(proxbundlemethodlog) << "value is " << _value_tp1 << ", gradient is " << gradient_tp1 << std::endl;
		LOG_DEBUG(proxbundlemethodlog) << "\tℒ(λ^(t+1)) = " << _bundleValue_tp1 << std::endl;
		LOG_DEBUG(proxbundlemethodlog) << "\tL(λ^(t+1)) = " << _value_tp1 << std::endl;
		LOG_DEBUG(proxbundlemethodlog) << "\teps_t      = " << _eps_t << std::endl;
	}

	LOG_DEBUG(proxbundlemethodlog) << "Maximum number of iterations reached -- aborting." << std::endl;
	_status = IterationsExceeded;

	return false;
}

template <typename ValueGradientCallback>
const std::vector<double>&
ProximalBundleMethod<ValueGradientCallback>:: getOptimalPosition() const {

	return _optimalPosition;
}

template <typename ValueGradientCallback>
double
ProximalBundleMethod<ValueGradientCallback>::getOptimalValue() const {

	return _optimalValue;
}

template <typename ValueGradientCallback>
void
ProximalBundleMethod<ValueGradientCallback>::setObjective() {

	// add the objective
	_objective = GRBQuadExpr(_slack);

	// l2 regularizer on current λ
	for (unsigned int i = 0; i < _numDims; i++) {

		// create λ_t - λ_t+1
		GRBLinExpr diff = _proxCenter_t[i] - _variables[i];

		// square it, multiply with regularizer weight
		GRBQuadExpr square = diff*diff*_regularizerWeight;

		_objective -= square;
	}

	_model.setObjective(_objective, GRB_MAXIMIZE);
	_model.update();
}

template <typename ValueGradientCallback>
void
ProximalBundleMethod<ValueGradientCallback>::updateProxCenter(const std::vector<double>& position, double value) {

	_proxCenter_t = position;
	_proxValue_t  = value;

	setObjective();
}

template <typename ValueGradientCallback>
bool
ProximalBundleMethod<ValueGradientCallback>::addHyperplane(const std::vector<double>& position, double value, const std::vector<double>& gradient_tp1) {

	// TODO:
	// • search in all planes, not just the last one
	if (_previous_gradient.size() > 0 && std::equal(_previous_gradient.begin(), _previous_gradient.end(), gradient_tp1.begin())) {

		LOG_DEBUG(proxbundlemethodlog) << "  trying to add previous hyperplane again -- skip it" << std::endl;

		return false;

	} else {

		_previous_gradient = gradient_tp1;

		LOG_DEBUG(proxbundlemethodlog) << "  adding new hyperplane" << std::endl;

		// offset b_i = ξ' - <g',x>
		double b = value - dot(gradient_tp1, position);

		// append g_i to A and b_i to b

		GRBLinExpr expr;
		expr.addTerms(&gradient_tp1[0], _variables, gradient_tp1.size());
		expr -= _slack;

		_model.addConstr(expr, GRB_GREATER_EQUAL, -b);
		_model.update();

		return true;
	}
}

template <typename ValueGradientCallback>
void
ProximalBundleMethod<ValueGradientCallback>::solveQP(std::vector<double>& position, double& value, double& qpValue) {

	// solve max ξ - regularizer, s.t. Ax - b ≤ ξ
	_model.optimize();

	int status = _model.get(GRB_IntAttr_Status);
	if (status != GRB_OPTIMAL) {

		LOG_DEBUG(proxbundlemethodlog) << "ERROR: bundle method could not find optimal value of ℒ(λ)!" << std::endl;

		UTIL_THROW_EXCEPTION(BundleCannotSolveQpException, "");
	}

	// λ^(t+1)
	for (unsigned int i = 0; i < _numDims; i++)
		position[i] = _variables[i].get(GRB_DoubleAttr_X);

	// ℒ(λ^(t+1))
	value = _slack.get(GRB_DoubleAttr_X);

	// ℒ(λ^(t+1)) - ρ/2*|λ'-λ|^2
	qpValue = _model.get(GRB_DoubleAttr_ObjVal);
}

template <typename ValueGradientCallback>
unsigned int
ProximalBundleMethod<ValueGradientCallback>::countNonzero(const std::vector<double>& values) {

	unsigned int nonzeros = 0;
	for (std::vector<double>::const_iterator i = values.begin(); i != values.end(); i++)
		if (*i != 0)
			nonzeros++;

	return nonzeros;
}

template <typename ValueGradientCallback>
double
ProximalBundleMethod<ValueGradientCallback>::dot(const std::vector<double>& a, const std::vector<double>& b) {

	double dotproduct = 0;
	std::vector<double>::const_iterator i,j;

	for (i = a.begin(), j = b.begin(); i != a.end(); i++, j++)
		dotproduct += (*i)*(*j);

	return dotproduct;
}

template <typename ValueGradientCallback>
void
ProximalBundleMethod<ValueGradientCallback>::getModelLog() {

	// get model statistics
	//_solverConstrs    = _model.get(GRB_IntAttr_NumConstrs);
	//_solverIterations = _model.get(GRB_IntAttr_IterCount);
	//_solverRuntime    = _model.get(GRB_DoubleAttr_Runtime);
}

template <typename ValueGradientCallback>
void
ProximalBundleMethod<ValueGradientCallback>::log() {

	// get statistics on the current λs
	//lambdaMin = None
	//lambdaMax = None
	//lambdaMean = 0
	//lambdaVar  = 0
	//for l in _lambda_tp1:
		//lambdaMean += l
		//if not lambdaMax or lambdaMax < l:
			//lambdaMax = l
		//if not lambdaMin or lambdaMin > l:
			//lambdaMin = l
	//lambdaMean /= len(_lambda_tp1)

	//for l in _lambda_tp1:
		//lambdaVar += (lambdaMean - l)**2
	//lambdaVar /= len(_lambda_tp1)

	//_steplength = linalg.norm(array(_lambda_tp1) - array(_lambda_t))
	//_nullsteplength = linalg.norm(array(_lambda_tp1) - array(_previous_proxCenter_t))
	//_serioussteplength = linalg.norm(array(_previous_proxCenter_t) - array(_proxCenter_t))
	//nonzeros = count_nonzero(_gradient_tp1)

	//_previous_proxCenter_t = _proxCenter_t

	//Logger.logBundleIteration(
			//_i,
			//_bundleValue_tp1,
			//_value_tp1,
			//_eps_t,
			//_improvement,
			//_steplength,
			//_nullsteplength,
			//_serioussteplength,
			//nonzeros,
			//lambdaMin,
			//lambdaMax,
			//lambdaMean,
			//lambdaVar,
			//_solverConstrs,
			//_solverIterations,
			//_solverRuntime)
}


#endif // HOST_INFERENCE_PROXIMAL_BUNDLE_METHOD_H__

