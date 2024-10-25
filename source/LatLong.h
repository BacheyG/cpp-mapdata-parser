#pragma once

struct LatLong {
public:
	LatLong();
	LatLong(double latitude, double longitude);
	bool Equals(const LatLong& other) const;
	double latitude;
	double longitude;
};