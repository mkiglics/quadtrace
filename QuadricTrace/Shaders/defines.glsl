#define CONE_TRACE_ALG cone_trace_gradient
#define PASS1_TRACING(ray, desc, inField) (quadricTrace(ray, desc, inField))
#define RAY_DIRECTIONS gRay6Directions
#define RAY_HALF_TANGENTS gRay6HalfTangents
#define UNBOUND_QUADRIC unboundQuadricConeTrace
