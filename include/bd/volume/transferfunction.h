//
// Created by Jim Pelton on 9/4/16.
//

#ifndef CRUFTERLY_TRANSFERFUNCTION_H
#define CRUFTERLY_TRANSFERFUNCTION_H

namespace bd
{

/// \brief A knot in an opacity transfer function
struct OpacityKnot
{
  double scalar;
  double alpha;
};

}
#endif //CRUFTERLY_TRANSFERFUNCTION_H
