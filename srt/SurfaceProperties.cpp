#include "SurfaceProperties.h"

namespace srt {

	Sellmeier3Function::Sellmeier3Function(Real const* bcs)
	{
		B1 = bcs[0];
		B2 = bcs[1];
		B3 = bcs[2];
		C1 = bcs[3];
		C2 = bcs[4];
		C3 = bcs[5];
	}
	Real Sellmeier3Function::operator()(Real lambda)
	{

		// nm -> micro metter
		lambda = 1E-3 * lambda;
		auto lambda2 = lambda * lambda;
		Real index = 1. + B1 * lambda2 / (lambda2 - C1)
			+ B2 * lambda2 / (lambda2 - C2)
			+ B3 * lambda2 / (lambda2 - C3);
		return sqrt(index);
	}


	std::shared_ptr<Sellmeier3Function> Sellmeier3(PredefinedSellmeier3 p)
	{
		Real BCs[] = {
			1.03961212,
			0.231792344,
			1.01046945,
			6.00069867E-3,
			2.00179144E-2,
			1.03560653E+2,
		};
		return std::make_shared<Sellmeier3Function>(BCs + 6 * (int)p);
	}

}

