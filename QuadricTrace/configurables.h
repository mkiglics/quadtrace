#pragma once

#include <vector>
#include <string>

namespace TraceTypes {
	// What kind of tracing we would like to use to render the frame
	struct TraceType
	{
		int id;
		std::string macro_value;
	};
	inline const TraceType sphere = {0, "(sphereTrace(ray, desc))"};
	inline const TraceType relaxed = {1, "(relaxedSphereTracing(ray, desc))"};
	inline const TraceType enhanced = {2, "(enhancedSphereTrace(ray, desc))"};
	inline const TraceType quadric = {3, "(quadricTrace(ray, desc, inField))"};	
	inline const std::vector<TraceType> traceTypes {
		TraceTypes::sphere, TraceTypes::relaxed, TraceTypes::enhanced, TraceTypes::quadric 
	};
}

namespace ConeTraceTypes {
	// What kind of cone tracing we would like to use if using quadric + cone
	struct ConeTraceType
	{
		int id;
		int rayCount;
	};
	inline const ConeTraceType tetrahedron = {0, 4};
	inline const ConeTraceType cube = {1, 6};
	inline const ConeTraceType octahedron = {2, 8};
	inline const ConeTraceType icosahedron = {3, 20};
	inline const std::vector<ConeTraceType> coneTraceTypes { 
		ConeTraceTypes::tetrahedron, ConeTraceTypes::cube, ConeTraceTypes::octahedron, ConeTraceTypes::icosahedron 
	};
}

bool operator==(const TraceTypes::TraceType& t1, const  TraceTypes::TraceType& t2);
bool operator==(const ConeTraceTypes::ConeTraceType& t1, const ConeTraceTypes::ConeTraceType& t2);
