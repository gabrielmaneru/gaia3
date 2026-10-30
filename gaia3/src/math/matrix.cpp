#include "gaia3_pch.h"
#include "matrix.h"



void Matrix33::SetRow(int row, const Vec3& v)
{
	switch (row)
	{
	case 0:
		col_0.x = v.x;
		col_1.x = v.y;
		col_2.x = v.z;
		break;
	case 1:
		col_0.y = v.x;
		col_1.y = v.y;
		col_2.y = v.z;
		break;
	case 2:
		col_0.z = v.x;
		col_1.z = v.y;
		col_2.z = v.z;
		break;
	default:
		assert(false);
		break;
	}
}



void Matrix44::SetRow(int row, const Vec4& v)
{
	switch (row)
	{
	case 0:
		col_0.x = v.x;
		col_1.x = v.y;
		col_2.x = v.z;
		col_3.x = v.w;
		break;
	case 1:
		col_0.y = v.x;
		col_1.y = v.y;
		col_2.y = v.z;
		col_3.y = v.w;
		break;
	case 2:
		col_0.z = v.x;
		col_1.z = v.y;
		col_2.z = v.z;
		col_3.z = v.w;
		break;
	case 3:
		col_0.w = v.x;
		col_1.w = v.y;
		col_2.w = v.z;
		col_3.w = v.w;
		break;
	default:
		assert(false);
		break;
	}
}