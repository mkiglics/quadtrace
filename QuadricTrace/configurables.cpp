#include "configurables.h"

bool operator==(const TraceTypes::TraceType& t1, const  TraceTypes::TraceType& t2)
{
	return t1.id == t2.id;
}

bool operator==(const ConeTraceTypes::ConeTraceType& t1, const ConeTraceTypes::ConeTraceType& t2)
{
	return t1.id == t2.id;
}

bool operator==(const ConeTraceTypes::ConeTraceAlgorithm& t1, const ConeTraceTypes::ConeTraceAlgorithm& t2)
{
	return t1.id == t2.id;
}