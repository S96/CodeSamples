#include "Terrain.h"


#include <cstdlib>
#include "SceneMngr.h"
#include "Scene.h"
#include "DeltA_Assets.h"
#include "DeltACamera3D.h"
#include "Collidable.h"
#include "GpuVertTypes.h"
#include "Visualizer.h"
#include "TextureTGA.h"
#include "TerrainIterator.h"
#include "CollisionVolumeBSphere.h"
#include "TerrainSettings.h"
#include "GraphicsObjectTerrain.h"


// START CONSTRUCTORS/DESTRUCTORS

Terrain::Terrain(TerrainSettings* t_settings) :
	_light_color(Color::White), _light_pos(0, t_settings->_max_y, 0), _n_side(t_settings->_n_side), 
	_min_y(t_settings->_min_y), _max_y(t_settings->_max_y), _model_update(false), _t_ptr(t_settings)
{
	_n_verts = _n_side * _n_side;
	_n_tris = (_n_side - 1) * (_n_side - 1) * 2;

	_verts = new VertexStride_VUN[_n_verts];
	_tris = new TriangleIndex[_n_tris];
	_cell_aabbs = new AABBCell[_n_tris / 2];

	_vert_spacing = t_settings->_width / float(_n_side - 1);
	_half_width = t_settings->_width / 2.0f;

	InitializeMesh(t_settings->_rep_u, t_settings->_rep_v);

	UpdateAllCellAABBs();

	UpdateAllNormals();

	CreateModel();
}

Terrain::Terrain(TerrainGenType t, TerrainSettings* t_settings, DiamondSquareSettings* ds_settings) :
	_light_color(Color::White), _light_pos(0, t_settings->_max_y, 0), _n_side(t_settings->_n_side),
	_min_y(t_settings->_min_y), _max_y(t_settings->_max_y), _model_update(false), _t_ptr(t_settings)
{
	if (_n_side % 2 == 0)
	{
		_n_side += 1;
	}

	_n_verts = _n_side * _n_side;
	_n_tris = (_n_side - 1) * (_n_side - 1) * 2;

	_verts = new VertexStride_VUN[_n_verts];
	_tris = new TriangleIndex[_n_tris];
	_cell_aabbs = new AABBCell[_n_tris / 2];

	_vert_spacing = t_settings->_width / float(_n_side - 1);
	_half_width = t_settings->_width / 2.0f;

	InitializeMesh(t_settings->_rep_u, t_settings->_rep_v);

	float range = (_max_y - _min_y);

	DeltAMath::ClampVect(ds_settings->_quad_seed, Vect(_min_y, _min_y, _min_y, _min_y), Vect(_max_y, _max_y, _max_y, _max_y));

	_verts[0].y = ds_settings->_quad_seed[x];
	_verts[_n_side - 1].y = ds_settings->_quad_seed[y];
	_verts[_n_verts - _n_side].y = ds_settings->_quad_seed[z];
	_verts[_n_verts - 1].y = ds_settings->_quad_seed[w];

	std::queue<DSData>* q_ptr = new std::queue<DSData>();

	// first run of method
	DSData data;
	data.Set(_n_side, ds_settings->_noise_magnitude, 0, 0);
	q_ptr->push(data);

	while (!q_ptr->empty())
	{
		data = q_ptr->front();
		q_ptr->pop();

		DiamondSquare(data.width, data.n_mag, data.x_ind, data.y_ind, ds_settings->_noise_reduction, q_ptr);
	}

	delete q_ptr;
	delete ds_settings;

	// Apply Gaussian smoothing to the entire mesh
	ApplyGaussian(1, _n_side - 2, 1, _n_side - 2);

	UpdateAllCellAABBs();

	UpdateAllNormals();

	CreateModel();
}

Terrain::Terrain(TerrainHMType t, TerrainSettings* t_settings, std::string hmap) :
	_light_color(Color::White), _light_pos(0, t_settings->_max_y, 0), _n_side(t_settings->_n_side),
	_min_y(t_settings->_min_y), _max_y(t_settings->_max_y), _model_update(false), _t_ptr(t_settings)
{
	InitializeFromHeightMap(t_settings->_rep_u, t_settings->_rep_v, t_settings->_width, hmap);

	UpdateAllCellAABBs();

	UpdateAllNormals();

	CreateModel();
}

Terrain::~Terrain()
{
	delete[] _verts;
	delete[] _tris;
	delete[] _cell_aabbs;
	delete _terrain_ptr;
	delete _go_tex_light;
	delete _t_ptr;
}

Terrain::AABBCell::AABBCell() : min_p(), max_p()
{

}

// END CONSTRUCTORS/DESTRUCTORS



// START ACCESSORS/MUTATORS

const float & Terrain::AABBCell::GetMin() const
{
	return min_p;
}

const float & Terrain::AABBCell::GetMax() const
{
	return max_p;
}

void Terrain::AABBCell::SetMin(const float & min)
{
	min_p = min;
}

void Terrain::AABBCell::SetMax(const float & max)
{
	max_p = max;
}

void Terrain::DSData::Set(int w, float mag, int x, int y)
{
	width = w;
	n_mag = mag;
	x_ind = x;
	y_ind = y;
}

// END ACCESSORS/MUTATORS



// START GENERAL METHODS

void Terrain::Render()
{
	_go_tex_light->Render(SceneMngr::GetCurrentScene()->GetCurrent3DCamera()->GetCamera());
}

void Terrain::InitializeMesh(float rep_u, float rep_v)
{
	float du = rep_u / (_n_side - 1);
	float dv = rep_v / (_n_side - 1);

	for (int i = 0; i < _n_side; i++)
	{
		for (int j = 0; j < _n_side; j++)
		{
			_verts[GetVertexIndex(j, i)].set(-_half_width + i * _vert_spacing, _min_y, -_half_width + j * _vert_spacing, j * du, i * dv, 0.0f, 0.0f, 0.0f);
		}
	}

	int triangle_index = 0;

	for (int i = 0; i < _n_side - 1; i++)
	{
		for (int j = 0; j < _n_side - 1; j++)
		{
			int p0 = i * _n_side + j,
				p1 = (i + 1) * _n_side + j + 1,
				p2 = p1 - 1;

			_tris[triangle_index].set(p0, p1, p2);
			++triangle_index;

			p2 = p0;
			p0 += 1;
			// no change to _p1

			_tris[triangle_index].set(p0, p1, p2);
			++triangle_index;
		}
	}
}

void Terrain::InitializeFromHeightMap(float rep_u, float rep_v, float width, std::string hmap)
{
	// load height map
	int i_width, i_height, i_comp;
	unsigned int d_type;

	std::string root = "Assets/Textures/";

	GLbyte* i_data = gltReadTGABits((root + hmap).c_str(), &i_width, &i_height, &i_comp, &d_type);
	assert(i_width == i_height && i_width != 0);

	_n_side = i_width;
	_n_verts = _n_side * _n_side;
	_n_tris = (_n_side - 1) * (_n_side - 1) * 2;

	_verts = new VertexStride_VUN[_n_verts];
	_tris = new TriangleIndex[_n_tris];
	_cell_aabbs = new AABBCell[_n_tris / 2];

	_vert_spacing = width / float(_n_side - 1);
	_half_width = width / 2.0f;

	float du = rep_u / (_n_side - 1);
	float dv = rep_v / (_n_side - 1);

	for (int i = 0; i < _n_side; i++)
	{
		for (int j = 0; j < _n_side; j++)
		{
			float y_val = (_max_y - _min_y) * static_cast<float>(static_cast<unsigned char>(*i_data) / 255.0f);
			
			_verts[GetVertexIndex(j, i)].set(-_half_width + i * _vert_spacing, _min_y + y_val, -_half_width + j * _vert_spacing, j * du, i * dv, 0.0f, 0.0f, 0.0f);

			i_data += 3;
		}
	}

	int triangle_index = 0;

	for (int i = 0; i < _n_side - 1; i++)
	{
		for (int j = 0; j < _n_side - 1; j++)
		{
			int p0 = i * _n_side + j,
				p1 = (i + 1) * _n_side + j + 1,
				p2 = p1 - 1;

			_tris[triangle_index++].set(p0, p1, p2);

			p0 += 1;
			// no change to _p1
			p2 = p0 - 1;

			_tris[triangle_index++].set(p0, p1, p2);
		}
	}
}

void Terrain::DiamondSquare(int width, float noise_mag, int x_ind, int y_ind, float noise_red, std::queue<DSData>* queue)
{
	int half_w = width / 2;

	// fetch four corner values
	// 1 - 2
	// |   |
	// 0 - 3
	Vect y_corners;

	y_corners[x] = _verts[GetVertexIndex(x_ind, y_ind)].y;
	y_corners[y] = _verts[GetVertexIndex(x_ind, y_ind + width - 1)].y;
	y_corners[z] = _verts[GetVertexIndex(x_ind + width - 1, y_ind + width - 1)].y;
	y_corners[w] = _verts[GetVertexIndex(x_ind + width - 1, y_ind)].y;

	// diamond step
	
	if (DeltAMath::IsNearZeroF(_verts[GetVertexIndex(x_ind + half_w, y_ind + half_w)].y - _min_y))
	{
		_verts[GetVertexIndex(x_ind + half_w, y_ind + half_w)].y = (y_corners[x] + y_corners[y] + y_corners[z] + y_corners[w]) / 4.0f /*+ DeltAMath::RandF(noise_mag, -noise_mag)*/;
	}

	noise_mag *= noise_red;

	// square step
	if (DeltAMath::IsNearZeroF(_verts[GetVertexIndex(x_ind, y_ind + half_w)].y - _min_y))
	{
		_verts[GetVertexIndex(x_ind, y_ind + half_w)].y = DeltAMath::ClampF((y_corners[x] + y_corners[y]) / 2.0f + DeltAMath::RandF(noise_mag, -noise_mag), _min_y, _max_y);
	}
	if (DeltAMath::IsNearZeroF(_verts[GetVertexIndex(x_ind + half_w, y_ind + half_w * 2)].y - _min_y))
	{
		_verts[GetVertexIndex(x_ind + half_w, y_ind + half_w * 2)].y = DeltAMath::ClampF((y_corners[y] + y_corners[z]) / 2.0f + DeltAMath::RandF(noise_mag, -noise_mag), _min_y, _max_y);
	}
	if (DeltAMath::IsNearZeroF(_verts[GetVertexIndex(x_ind + half_w * 2, y_ind + half_w)].y - _min_y))
	{
		_verts[GetVertexIndex(x_ind + half_w * 2, y_ind + half_w)].y = DeltAMath::ClampF((y_corners[z] + y_corners[w]) / 2.0f + DeltAMath::RandF(noise_mag, -noise_mag), _min_y, _max_y);
	}
	if (DeltAMath::IsNearZeroF(_verts[GetVertexIndex(x_ind + half_w, y_ind)].y - _min_y))
	{
		_verts[GetVertexIndex(x_ind + half_w, y_ind)].y = DeltAMath::ClampF((y_corners[w] + y_corners[x]) / 2.0f + DeltAMath::RandF(noise_mag, -noise_mag), _min_y, _max_y);
	}

	noise_mag *= noise_red;
	
	if (width > 3) // base case
	{
		DSData data;
		data.n_mag = noise_mag;
		data.width = half_w + 1;

		data.x_ind = x_ind;
		data.y_ind = y_ind;
		queue->push(data);

		data.x_ind = x_ind + half_w;
		data.y_ind = y_ind;
		queue->push(data);

		data.x_ind = x_ind;
		data.y_ind = y_ind + half_w;
		queue->push(data);

		data.x_ind = x_ind + half_w;
		data.y_ind = y_ind + half_w;
		queue->push(data);
	}
}

void Terrain::ApplyGaussian(int x_min, int x_max, int y_min, int y_max)
{
	for (int i = y_min; i < y_max + 1; ++i)
	{
		for (int j = x_min; j < x_max + 1; ++j)
		{
			GaussianBlur(j, i);
		}
	}
}

void Terrain::GaussianBlur(int x_ind, int y_ind)
{
	// (0,0)
	float y_val = _verts[GetVertexIndex(x_ind, y_ind)].y * DeltAMath::GAUSSIAN_0_0;

	// (-1,0)
	x_ind -= 1;
	y_val += _verts[GetVertexIndex(x_ind, y_ind)].y * DeltAMath::GAUSSIAN_0_1;
	
	// (-1,-1)
	y_ind -= 1;
	y_val += _verts[GetVertexIndex(x_ind, y_ind)].y * DeltAMath::GAUSSIAN_1_1;

	// (-1,1)
	y_ind += 2;
	y_val += _verts[GetVertexIndex(x_ind, y_ind)].y * DeltAMath::GAUSSIAN_1_1;

	// (0,1)
	x_ind += 1;
	y_val += _verts[GetVertexIndex(x_ind, y_ind)].y * DeltAMath::GAUSSIAN_0_1;

	// (0,-1)
	y_ind -= 2;
	y_val += _verts[GetVertexIndex(x_ind, y_ind)].y * DeltAMath::GAUSSIAN_0_1;

	// (1,-1)
	x_ind += 1;
	y_val += _verts[GetVertexIndex(x_ind, y_ind)].y * DeltAMath::GAUSSIAN_1_1;

	// (1,0)
	y_ind += 1;
	y_val += _verts[GetVertexIndex(x_ind, y_ind)].y * DeltAMath::GAUSSIAN_0_1;

	// (1,1)
	y_ind += 1;
	y_val += _verts[GetVertexIndex(x_ind, y_ind)].y * DeltAMath::GAUSSIAN_1_1;

	_verts[GetVertexIndex(x_ind - 1, y_ind - 1)].y = y_val;
}

void Terrain::DeformSpherical(const Vect & pos, float radius, float intensity)
{
	int size;
	int* indices = VertsInRange(pos, radius, size);

	for (int i = 0; i < size; i++)
	{
		float delta_y = DeltaYHemisphere(pos, _verts[indices[i]].x, _verts[indices[i]].z, radius) * intensity;

		_verts[indices[i]].y -= delta_y;
		_verts[indices[i]].txt += fabsf(delta_y);
	}

	int xmin, xmax, ymin, ymax;

	// apply Gaussian smoothing to altered verts
	AffectedVertRange(indices[0], indices[size - 1], xmin, xmax, ymin, ymax);
	ApplyGaussian(xmin, xmax, ymin, ymax);

	// Update CellAABBs
	AffectedCellRange(indices[0], indices[size - 1], xmin, xmax, ymin, ymax);
	UpdateCellAABBs(xmin, ymin, xmax, ymax);

	UpdateAllNormals();

	delete[] indices;

	SetModelUpdate(true);
}

bool Terrain::GetPoint(const Vect & pos, Vect & out)
{
	int t_ind = GetTriangleIndex(pos);

	bool ret_val = false;

	if (InRangeTriangle(t_ind))
	{
		VertexStride_VUN vert = _verts[_tris[t_ind].v0];
		Vect p0 = Vect(vert.x, vert.y, vert.z);

		vert = _verts[_tris[t_ind].v1];
		Vect p1 = Vect(vert.x, vert.y, vert.z);

		vert = _verts[_tris[t_ind].v2];
		Vect p2 = Vect(vert.x, vert.y, vert.z);

		ret_val = DeltAMath::LinePlaneIntersect((p1 - p0).cross(p2 - p0), p0, Vect(0, -1, 0), pos, out);
	}

	return ret_val;
}

bool Terrain::GetFaceNormal(const Vect & pos, Vect & out)
{
	int t_ind = GetTriangleIndex(pos);

	bool ret_val = false;

	if (InRangeTriangle(t_ind))
	{
		VertexStride_VUN vert = _verts[_tris[t_ind].v0];
		Vect p0 = Vect(vert.x, vert.y, vert.z);

		vert = _verts[_tris[t_ind].v1];
		Vect p1 = Vect(vert.x, vert.y, vert.z);

		vert = _verts[_tris[t_ind].v2];
		Vect p2 = Vect(vert.x, vert.y, vert.z);

		out = (p1 - p0).cross(p2 - p0);

		ret_val = true;
	}

	return ret_val;
}