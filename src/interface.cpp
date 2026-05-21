#include "interface.h"

void Interface::connect(std::shared_ptr<Link> link) {
    connectedLink = link;
}