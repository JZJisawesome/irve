/* common.cpp
 * Copyright (C) TODO John Jekel
 * See the LICENSE file at the root of the project for licensing info.
 *
 * Function implementations for common.h
 *
*/

/* Includes */

#include "common.h"

/* Function Implementations */

word_t::word_t() {}

word_t::word_t(uint32_t u) : u(u) {}

word_t::word_t(int32_t s) : s(s) {}

word_t word_t::signed_negate() const {
    return word_t(-this->s);
}

word_t word_t::operator+(const word_t& other) const {
    return word_t(this->u + other.u);
}

word_t& word_t::operator+=(const word_t& other) {
    return *this = *this + other;
}

word_t& word_t::operator++() {
    return *this += 1;
}

word_t word_t::operator-(const word_t& other) const {
    //TODO ensure this is the same for signed
    return word_t(this->u - other.u);
}

word_t& word_t::operator-=(const word_t& other) {
    return *this = *this - other;
}

word_t& word_t::operator--() {
    return *this -= 1;
}

word_t word_t::operator*(const word_t& other) const {
    return word_t(this->u * other.u);
}

word_t& word_t::operator*=(const word_t& other) {
    return *this = *this * other;
}

word_t word_t::operator~() const {
    return word_t(~this->u);
}

word_t word_t::operator|(const word_t& other) const {
    return word_t(this->u | other.u);
}

word_t& word_t::operator|=(const word_t& other) {
    return *this = *this | other;
}

word_t word_t::operator&(const word_t& other) const {
    return word_t(this->u & other.u);
}

word_t& word_t::operator&=(const word_t& other) {
    return *this = *this & other;
}

word_t word_t::operator^(const word_t& other) const {
    return word_t(this->u ^ other.u);
}

word_t& word_t::operator^=(const word_t& other) {
    return *this = *this ^ other;
}

word_t word_t::operator<<(const word_t& other) const {
    return word_t(this->u << other.u);
}

word_t& word_t::operator<<=(const word_t& other) {
    return *this = *this << other;
}

word_t word_t::srl(const word_t& other) const {
    return word_t(this->u >> other.u);
}

word_t word_t::sra(const word_t& other) const {
    //TODO ensure this is arithmetic shift
    return word_t(this->s >> other.s);
}

bool word_t::operator!() const {
    return this->u == 0;
}

bool word_t::operator==(const word_t& other) const {
    return this->u == other.u;
}

bool word_t::operator!=(const word_t& other) const {
    return !(*this == other);
}
