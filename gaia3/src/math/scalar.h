#pragma once

struct Scalar
{
	explicit Scalar(float _value) : value(_value) {}
	operator float() const { return value; }

	float value;
};
