

struct Vec2 {
    Real fX;
    Real fY;
};

struct FaceVertex {
    int VtxIdx;
    int VtxTextureIdx = -1;
    int VtxNormalIdx = -1;
};

struct Face {
    std::vector<FaceVertex> Vtxs;
};

struct Obj {

    std::vector<Vec3> Vtxs;
    std::vector<Vec3> VtxNormals;
    std::vector<Vec2> VtxTextures;
    std::vector<Face> Faces;
    std::string material;


    bool loadOBJ(const char* filepath);
};


struct Triangle : Device, SurfaceProperties
{

    static bool intersect_triangle(
        Vec3 const& Origin,
        Vec3 const& Dir,
        Vec3 const& A,
        Vec3 const& B,
        Vec3 const& C,
        Real& t,
        Vec3& N)
    {
        Real u, v;
        Vec3 E1 = B - A;
        Vec3 E2 = C - A;
        N = cross(E1, E2);
        Real det = -dot(Dir, N);
        Vec3 AO = Origin - A;
        Real AON = dot(AO, N);

        if (det * AON <= 0) return false;

        Real invdet = 1.0 / det;
        t = AON * invdet;

        Vec3 DAO = cross(AO, Dir);
        u = dot(E2, DAO) * invdet;
        v = -dot(E1, DAO) * invdet;
        return (u >= 0.0 && v >= 0.0 && (u + v) <= 1.0);
    }


    void process(Ray const& ray,
        ProcessHandler& handler) override
    {
        Real s;
        Vec3 N;
        if (!intersect_triangle(ray.fO, ray.fD,
            fP1, fP2, fP3, s, N)) {
            return;
        }

        if (handler.fType == HandlerType::Distance) {
            static_cast<DistanceHandler&>(handler).distance(s);
        }

        N = normalize(N);
        Vec3 inter = ray.fO + ray.fD * s;

        static_cast<TracingHandler&>(handler).surface(inter,
            N, dot(N, ray.fD) > 0, this);
    }

private:
    Vec3 fP1;
    Vec3 fP2;
    Vec3 fP3;
};