#pragma once

namespace TraceTypes {
	// What kind of tracing we would like to use to render the frame
	struct TraceType
	{
		int id;
		std::string function_name;
	};
	inline const TraceType sphere = {0, "sphereTrace"};
	inline const TraceType relaxed = {1, "relaxedSphereTracing"};
	inline const TraceType enhanced = {2, "enhancedSphereTrace"};
	inline const TraceType quadric = {3, "quadricTrace"};	
	inline const std::vector<const TraceType> traceTypes {
		TraceTypes::sphere, TraceTypes::relaxed, TraceTypes::enhanced, TraceTypes::quadric 
	};

	bool operator==(const TraceType& t1, const TraceType& t2) 
	{
		return t1.id == t2.id;
	}
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
	inline const std::vector<const ConeTraceType> coneTraceTypes { 
		ConeTraceTypes::tetrahedron, ConeTraceTypes::cube, ConeTraceTypes::octahedron, ConeTraceTypes::icosahedron 
	};

	bool operator==(const ConeTraceType& t1, const ConeTraceType& t2) 
	{
		return t1.id == t2.id;
	}
}
