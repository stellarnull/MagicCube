class Cube
{
public:
	int posID; //ÿ������Ψһ��Ӧ��ID
	float points[9][3]; //��������ĺͰ˸����������
	bool outerSurface[6]; //ÿ�����Ƿ�������棨�Ƿ���Ҫ��ͼ)
	int surfaceType[6]; //ÿ�����Ӧ����ͼ��ʽ
};

void InitializeCubes(Cube * cubes);
int processHits (GLint hits, GLuint selectBuff[]);