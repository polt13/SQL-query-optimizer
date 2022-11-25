#include "intmd_r.h"
//#include "intmd_r.h"

intmd_r::intmd_r(int64_t s) : size{s}, arr{new LinkedList[s]{}} {}

int64_t intmd_r::getSize() const { return this->size; }