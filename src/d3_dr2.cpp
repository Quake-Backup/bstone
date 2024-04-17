/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Former "D3_DR_A2.ASM"
//
// Define "W3D" to emulate "D3_DR_A.ASM".

#include <cmath>

#include <vector>

#include "3d_def.h"
#include "id_vl.h"

void HitHorizWall();
void HitVertWall();
void HitHorizDoor();
void HitVertDoor();
void HitHorizPWall();
void HitVertPWall();

extern FineTangent finetangent;
extern int viewwidth;
extern TileMap tilemap;
extern SpotVis spotvis;
extern std::vector<int> pixelangle;
extern int midangle;
extern int focaltx;
extern int focalty;
extern int viewtx;
extern int viewty;
extern double viewx;
extern double viewy;
extern double xpartialup;
extern double xpartialdown;
extern double ypartialup;
extern double ypartialdown;
extern int tilehit;
extern int pixx;
extern WallHeight wallheight;
extern int xtile;
extern int ytile;
extern int xtilestep;
extern int ytilestep;
extern double xintercept;
extern double yintercept;
double xstep;
double ystep;
extern DoorPositions doorposition;
extern double pwallpos;

namespace {

constexpr auto DEG90 = std::intptr_t{900};
constexpr auto DEG180 = std::intptr_t{1800};
constexpr auto DEG270 = std::intptr_t{2700};
constexpr auto DEG360 = std::intptr_t{3600};

double get_integral_1(double value) noexcept
{
	return value < 0 ? std::floor(value) : std::trunc(value);
}

double get_fractional_1(double value) noexcept
{
	double integral;
	return std::modf(1 + value, &integral);
}

} // namespace

void AsmRefresh()
{
	double xpartial;
	double ypartial;
	int xs_x; // temporary xspot (x)
	int xs_y; // temporary xspot (y)
	int xt; // temporary xtile
	int xint_h; // high part of temporary xintercept
	int ys_x; // temporary yspot (x)
	int ys_y; // temporary yspot (y)
	int yt; // temporary ytile
	int yint_h; // high part of temporary yintercept

	vid_hw_clear_wall_render_list();
	vid_hw_clear_pushwall_render_list();
	vid_hw_clear_door_render_list();

	pixx = 0;

pixxloop:

	auto angle = midangle + pixelangle[pixx];

	while (angle < 0)
	{
		angle += DEG360;
	}

	while (angle >= DEG360)
	{
		angle -= DEG360;
	}

	if (angle >= DEG270)
	{
		xtilestep = 1;
		ytilestep = 1;

		xstep = finetangent[angle - DEG270];
		ystep = finetangent[DEG360 - 1 - angle];

		xpartial = xpartialup;
		ypartial = ypartialup;
	}
	else if (angle >= DEG180)
	{
		xtilestep = -1;
		ytilestep = 1;

		xstep = -finetangent[DEG270 - 1 - angle];
		ystep = finetangent[angle - DEG180];

		xpartial = xpartialdown;
		ypartial = ypartialup;
	}
	else if (angle >= DEG90)
	{
		xtilestep = -1;
		ytilestep = -1;

		xstep = -finetangent[angle - DEG90];
		ystep = -finetangent[DEG180 - 1 - angle];

		xpartial = xpartialdown;
		ypartial = ypartialdown;
	}
	else
	{
		xtilestep = 1;
		ytilestep = -1;

		xstep = finetangent[DEG90 - 1 - angle];
		ystep = -finetangent[angle];

		xpartial = xpartialup;
		ypartial = ypartialdown;
	}


	//
	// initialise variables for intersection testing
	//

	yintercept = viewy + ystep * xpartial;
	yint_h = static_cast<int>(yintercept);

	xtile = focaltx + xtilestep;
	xs_x = xtile;
	xs_y = yint_h;

	const auto xint_1 = viewx + xstep * ypartial;
	xint_h = static_cast<int>(get_integral_1(xint_1));

	xintercept = get_fractional_1(xint_1);

	yt = focalty + ytilestep;
	ys_x = xint_h;
	ys_y = yt;

	xt = xtile;

	// trace along this angle until we hit a wall
	//
	// CORE LOOP!

	// check intersections with vertical walls

vertcheck:

	if ((ytilestep < 0 && yint_h <= yt) || (ytilestep > 0 && yint_h >= yt))
	{
		goto horizentry;
	}

vertentry:

	if (tilemap[xs_x][xs_y] != 0)
	{
		tilehit = tilemap[xs_x][xs_y];

		if ((tilehit & 0x80) != 0)
		{
			// hit a special vertical wall, so find which coordinate a
			// door would be intersected at, and check to see if the
			// door is open past that point

			xtile = xt;
			yintercept = yint_h + get_fractional(yintercept);

			if ((tilehit & 0x40) != 0)
			{
				// hit a sliding vertical wall

				const auto intercept = ystep * pwallpos;
				const auto yint = get_fractional(intercept) + get_fractional(yintercept);
				const auto intercept_l = get_fractional_1(yint); // low part of intercept
				// high part of intercept
				const auto intercept_h = yint_h + static_cast<int>(intercept + get_integral_1(yint));

				if (yint_h == intercept_h)
				{
					// draw the pushable wall at the new height

					yintercept = intercept_h + intercept_l;
					xintercept = xt;

					HitVertPWall();

					goto nextpix;
				}
			}
			else
			{
				const auto intercept = yintercept + 0.5 * ystep;
				const auto intercept_l = get_fractional(intercept); // low part of intercept
				const auto intercept_h = static_cast<int>(intercept); // high part of intercept

				if (static_cast<int>(yintercept) == intercept_h)
				{
					// the trace hit the door plane at pixel position,
					// see if the door is closed that much
					//
					// draw the door

					const auto doorpos = tilehit & 0x7F;
					auto skip_draw = false;

#ifndef W3D
					if (intercept_l < 0.5)
					{
						if (2 * intercept_l > 1 - doorposition[doorpos])
						{
							skip_draw = true;
						}
					}
					else
					{
						if (2 * intercept_l < 1 + doorposition[doorpos])
						{
							skip_draw = true;
						}
					}
#else
					if (intercept_l < doorposition[doorpos])
					{
						skip_draw = true;
					}
#endif // W3D

					if (!skip_draw)
					{
						//
						// draw the trek door
						//

						yintercept = intercept_l + get_integral(yintercept);
						xintercept = xtile + 0.5;

						HitVertDoor();

						goto nextpix;
					}
				}
			}

			// midpoint is outside tile,
			// so it hit the side of the wall before a door

			xt = xtile;
			yint_h = static_cast<int>(yintercept);
		}
		else
		{
			xintercept = xt;
			xtile = xt;

			yintercept = yint_h + get_fractional(yintercept);
			ytile = yint_h;

			HitVertWall();

			goto nextpix;
		}
	}

	spotvis[xs_x][xs_y] = 1;

	xt += xtilestep;

	const auto yint_1 = get_fractional(yintercept) + get_fractional(ystep);
	yintercept = get_integral(yintercept) + get_fractional_1(yint_1);

	yint_h += static_cast<int>(ystep + get_integral_1(yint_1));

	xs_x = xt;
	xs_y = yint_h;

	goto vertcheck;


	// check intersections with horizontal walls

horizcheck:

	if ((xtilestep > 0 && xint_h >= xt) || (xtilestep < 0 && xint_h <= xt))
	{
		goto vertentry;
	}

horizentry:

	if (tilemap[ys_x][ys_y] != 0)
	{
		tilehit = tilemap[ys_x][ys_y];

		if ((tilehit & 0x80) != 0)
		{
			// hit a special horizontal wall, so find which coordinate a
			// door would be intersected at, and check to see if the
			// door is open past that point

			xtile = xt;

			yintercept = yint_h + get_fractional(yintercept);

			if ((tilehit & 0x40) != 0)
			{
				// hit a sliding horizontal wall

				const auto intercept = xstep * pwallpos;
				const auto xint = get_fractional(intercept) + get_fractional(xintercept);
				const auto intercept_l = get_fractional_1(xint); // low part of intercept
				// high part of intercept
				const auto intercept_h = xint_h + static_cast<int>(intercept + get_integral_1(xint));

				if (xint_h == intercept_h)
				{
					// draw the pushable wall at the new height

					xintercept = intercept_h + intercept_l;
					yintercept = yt;

					HitHorizPWall();

					goto nextpix;
				}
			}
			else
			{
				const auto intercept = 0.5 * xstep;
				const auto xint = get_fractional(intercept) + get_fractional(xintercept);
				const auto intercept_l = get_fractional_1(xint); // low part of intercept
				// high part of intercept
				const auto intercept_h = xint_h + static_cast<int>(intercept + get_integral_1(xint));

				if (xint_h == intercept_h)
				{
					// the trace hit the door plane at pixel position,
					// see if the door is closed that much
					//
					// draw the door

					const auto doorpos = tilehit & 0x7F;
					auto skip_draw = false;

#ifndef W3D
					if (intercept_l < 0.5)
					{
						if (2 * intercept_l > 1 - doorposition[doorpos])
						{
							skip_draw = true;
						}
					}
					else
					{
						if (2 * intercept_l < 1 + doorposition[doorpos])
						{
							skip_draw = true;
						}
					}
#else
					if (intercept_l < doorposition[doorpos])
					{
						skip_draw = true;
					}
#endif // W3D

					if (!skip_draw)
					{
						// draw the trek door

						xintercept = xint_h + intercept_l;
						yintercept = yt + 0.5;

						HitHorizDoor();

						goto nextpix;
					}
				}
			}


			// midpoint is outside tile,
			// so it hit the side of the wall before a door

			xt = xtile;
			yint_h = static_cast<int>(yintercept);
		}
		else
		{
			xintercept = xint_h + get_fractional(xintercept);
			xtile = xint_h;

			yintercept = yt;
			ytile = yt;

			HitHorizWall();

			goto nextpix;
		}
	}

	spotvis[ys_x][ys_y] = 1;

	yt += ytilestep;

	const auto xint_2 = get_fractional(xintercept) + get_fractional(xstep);
	xintercept = get_integral(xintercept) + get_fractional_1(xint_2);

	xint_h += static_cast<int>(xstep + get_integral_1(xint_2));

	ys_x = xint_h;
	ys_y = yt;

	goto horizcheck;


	// next pixel over

nextpix:
	++pixx;

	if (pixx < viewwidth)
	{
		goto pixxloop;
	}
}
