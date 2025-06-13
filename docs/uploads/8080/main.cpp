#include <iostream>
#include "calc.hpp"

int main() {
	CCalc *pC1, *pC2;

	pC1 = new CCalc();
	pC2 = new CCalc(1, 2);
	std::cout << 3 << " + " << 4 << " = " << pC1->add(3, 4) << std::endl;
	std::cout << pC2->getA() << " + " << pC2->getB() << " = " << pC2->add() << std::endl;
	delete pC1;
	delete pC2;
	return 0;
}

