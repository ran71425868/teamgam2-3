#include "Card.h"

Card::Card(std::string n, CardType t, std::string e, int id, bool persistent) : name(n), type(t), effect(e), effectId(id), isPersistent(persistent)
{
}
