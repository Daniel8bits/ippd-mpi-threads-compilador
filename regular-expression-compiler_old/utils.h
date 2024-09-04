#ifndef UTILS_H
#define UTILS_H

template<class T1, class T2>
class Pair {
public:
  Pair(T1 first, T2 second): first(first), second(second) {};
  T1 first;
  T2 second;
};

#endif // UTILS_H
