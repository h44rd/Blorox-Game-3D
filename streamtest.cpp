#include <iostream>
#include <sstream>
using namespace std;

int main () {

  int val=65;
  stringstream ss (stringstream::in | stringstream::out);

  ss << val;

  char hello[100];
  strcpy(hello,"Hello: "):
  strcat(hello,ss.str().c_str());
  cout<<hello;

  return 0;
}
