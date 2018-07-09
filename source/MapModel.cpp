#include "quake/MapModel.h"

#include <SM_Calc.h>

namespace
{

const sm::vec3 BASE_AXES[] =
{
    sm::vec3( 0.0f,  0.0f,  1.0f), sm::vec3( 1.0f,  0.0f,  0.0f), sm::vec3( 0.0f, -1.0f,  0.0f),
    sm::vec3( 0.0f,  0.0f, -1.0f), sm::vec3( 1.0f,  0.0f,  0.0f), sm::vec3( 0.0f, -1.0f,  0.0f),
    sm::vec3( 1.0f,  0.0f,  0.0f), sm::vec3( 0.0f,  1.0f,  0.0f), sm::vec3( 0.0f,  0.0f, -1.0f),
    sm::vec3(-1.0f,  0.0f,  0.0f), sm::vec3( 0.0f,  1.0f,  0.0f), sm::vec3( 0.0f,  0.0f, -1.0f),
    sm::vec3( 0.0f,  1.0f,  0.0f), sm::vec3( 1.0f,  0.0f,  0.0f), sm::vec3( 0.0f,  0.0f, -1.0f),
    sm::vec3( 0.0f, -1.0f,  0.0f), sm::vec3( 1.0f,  0.0f,  0.0f), sm::vec3( 0.0f,  0.0f, -1.0f),
};

}

namespace quake
{

//////////////////////////////////////////////////////////////////////////
// class BrushFace
//////////////////////////////////////////////////////////////////////////

void BrushFace::SortVertices()
{
	sm::vec3 center;
	for (auto& v : vertices) {
		center += v;
	}
	center /= static_cast<float>(vertices.size());

	for (size_t i = 0; i < vertices.size() - 2; ++i)
	{
		float smallest_angle = -1;
		int   smallest_idx = -1;

		sm::vec3 a = (vertices[i] - center).Normalized();
		sm::Plane p(vertices[i], center, center + plane.normal);
		for (size_t j = i + 1; j < vertices.size(); ++j)
		{
			float dis = p.normal.Dot(vertices[j]) + p.dist;
			// black
			if (dis < -SM_LARGE_EPSILON) {
				;
			} else {
				sm::vec3 b = (vertices[j] - center).Normalized();
				float angle = a.Dot(b);
				if (angle > smallest_angle) {
					smallest_angle = angle;
					smallest_idx = j;
				}
			}
		}

		if (smallest_idx == -1 )
		{
			assert(0);
			//std::cout << "Error: Degenerate polygon!" << std::endl
			//abort ( );
		}

		if (smallest_idx != i + 1) {
			std::swap(vertices[smallest_idx], vertices[i + 1]);
		}
	}

	//// fix back faces
	//assert(vertices.size() > 2);
	//sm::vec3 normal = (vertices[2] - vertices[0]).Cross(vertices[1] - vertices[0]).Normalized();
	//if (normal.Dot(plane.normal) < -SM_LARGE_EPSILON) {
	//	std::reverse(std::begin(vertices), std::end(vertices));
	//}
}

void BrushFace::InitTexCoordSys()
{
	sm::vec3 normal = (vertices[2] - vertices[0]).Cross(vertices[1] - vertices[0]).Normalized();

	size_t best_idx = 0;
	float  best_dot = 0;
	for (size_t i = 0; i < 6; ++i)
	{
		float dot = normal.Dot(BASE_AXES[i * 3]);
		if (dot > best_dot) {
			best_dot = dot;
			best_idx = i;
		}
	}

	tc_sys.index = best_idx;
	tc_sys.x_axis = BASE_AXES[best_idx * 3 + 1];
	tc_sys.y_axis = BASE_AXES[best_idx * 3 + 2];

	// fix back faces
	assert(vertices.size() > 2);
	auto& z_axis = BASE_AXES[tc_sys.index * 3 + 0];
	// todo: mirror x axis, so normal should be opposite direction
	//if (normal.Dot(z_axis) < -SM_LARGE_EPSILON) {
	if (normal.Dot(/*z_axis*/normal) > SM_LARGE_EPSILON) {
		std::reverse(std::begin(vertices), std::end(vertices));
	}
}

sm::vec2 BrushFace::CalcTexCoords(const sm::vec3& pos, float tex_w, float tex_h) const
{
	sm::vec2 ret;

	float sx = scale.x, sy = scale.y;
	if (fabs(sx) < std::numeric_limits<float>::epsilon()) {
		sx = 1;
	}
	if (fabs(sy) < std::numeric_limits<float>::epsilon()) {
		sy = 1;
	}

	ret.x = (pos.Dot(tc_sys.x_axis / sx) + offset.x) / tex_w;
	ret.y = (pos.Dot(tc_sys.y_axis / sy) + offset.y) / tex_h;

	return ret;
}

void BrushFace::AddVertex(const sm::vec3& v)
{
	for (auto& vert : vertices) {
		if (sm::dis_square_pos3_to_pos3(vert, v) < SM_LARGE_EPSILON) {
			return;
		}
	}
	vertices.push_back(v);
}

//////////////////////////////////////////////////////////////////////////
// class MapBrush
//////////////////////////////////////////////////////////////////////////

void MapBrush::BuildVertices()
{
	// create vertices
	int n = faces.size();
	for (int i = 0; i < n - 2; ++i) {
		for (int j = i + 1; j < n - 1; ++j) {
			for (int k = j + 1; k < n; ++k) {
				sm::vec3 v;
				if (!sm::intersect_planes(faces[i].plane, faces[j].plane, faces[k].plane, &v)) {
					continue;
				}

				// Test if the point is outside the brush
				bool legal = true;
				for (auto& f : faces) {
					// black
					if (f.plane.normal.Dot(v) + f.plane.dist < -SM_LARGE_EPSILON) {
						legal = false;
						break;
					}
				}

				if (legal)
				{
					faces[i].AddVertex(v);
					faces[j].AddVertex(v);
					faces[k].AddVertex(v);
				}
			}
		}
	}

	// sort vertices
	for (auto& f : faces) {
		assert(f.vertices.size() >= 3);
		f.SortVertices();
		f.InitTexCoordSys();
	}
}

}