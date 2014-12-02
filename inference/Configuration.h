#ifndef HOST_INFERENCE_CONFIGURATION_H__
#define HOST_INFERENCE_CONFIGURATION_H__

struct Configuration {

	/**
	 * Consider lambdas as zero if they are smaller than this value.
	 */
	static constexpr double LambdaEpsilon = 1e-6;
};

#endif // HOST_INFERENCE_CONFIGURATION_H__

