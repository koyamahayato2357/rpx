#include "phyconst.h"
#include "arthfn.h"
#include "chore.h"
#include "testing.h"

double const consts[ALPN * 2] = {
  0,               // A
  0,               // B
  0,               // C
  0,               // D
  M_E,             // E Eular's number
  96485.33212331,  // F Faraday constant
  6.6743015e-11,   // G
  0,               // H
  0,               // I
  0,               // J
  0,               // K
  6.02214076e23,   // L Avogadro constant
  0,               // M
  0,               // N
  0,               // O
  M_PI,            // P pie
  0,               // Q
  8.3144626181532, // R gas constant
  0,               // S
  0,               // T
  0,               // U
  0,               // V
  0,               // W
  0,               // X
  0,               // Y
  0,               // Z

  0,               // a
  0,               // b
  299'792'458,     // c speed of light
  0,               // d
  1.602176634e-19, // e elementary charge
  0,               // f
  0,               // g
  6.62607015e-34,  // h Planck constant
  0,               // i
  0,               // j
  1.380649e-23,    // k Boltzmann constant
  0,               // l
  0,               // m
  0,               // n
  0,               // o
  0,               // p
  0,               // q
  0,               // r
  0,               // s
  0,               // t
  0,               // u
  0,               // v
  0,               // w
  0,               // x
  0,               // y
  0,               // z
};

double get_const(char idx) {
  return consts[idx - 'A' - (idx >= 'a') * ('a' - 'Z' - 1)];
}

test_table(
  get_const, get_const, (double, char),
  {
    {    299'792'458, 'c'},
    {           M_PI, 'P'},
    {8.3144626181532, 'R'},
    {            M_E, 'E'},
    {   1.380649e-23, 'k'}
}
)
