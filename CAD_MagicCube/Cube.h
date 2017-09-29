class Cube
{
public:
	int posID; //每个方块唯一对应的ID
	float points[9][3]; //方块的中心和八个顶点的坐标
	bool outerSurface[6]; //每个面是否是外表面（是否需要贴图)
	int surfaceType[6]; //每个面对应的贴图样式
};

void InitializeCubes(Cube * cubes);
int processHits (GLint hits, GLuint selectBuff[]);