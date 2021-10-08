#pragma once


class GameSprite {
public:
	GameSprite(HINSTANCE instance, UINT resource, UINT number  = 1, int xoff = 0, int yoff = 0);
	GameSprite(LPCTSTR path, UINT number = 1, int xoff = 0, int yoff = 0);
	~GameSprite();

	void draw(HDC surface, double x, double y, double index = 0.0, double angle = 0.0, double xscale = 1.0, double yscale = 1.0, double alpha = 1.0);

	void set_bbox(const LONG left, const LONG right, const LONG top, const LONG bottom);

	const int get_width() const;
	const int get_height() const;

	const u_int number; // �̹��� ���
	const int xoffset, yoffset; // ��������Ʈ�� �߽���
	RECT bbox;

private:
	bool __process_image(CImage&, const size_t = 0, const size_t = 0);
	void __draw_single(HDC, CImage&, const double, const double, const double = 0.0, const double = 1.0, const double = 1.0, const double = 1.0);

	CImage raw; // ���� �׸�.
	size_t raw_width, raw_height; // ���� ũ��

	vector<unique_ptr<CImage>> frames; // �߸� �׸�. ������� ���� �ִ�.
	size_t width, height; // ������ ũ��

	//BLENDFUNCTION blender;
};

shared_ptr<GameSprite> make_sprite(HINSTANCE instance, UINT resource, UINT number = 1, int xoff = 0, int yoff = 0);

shared_ptr<GameSprite> make_sprite(LPCTSTR path, UINT number = 1, int xoff = 0, int yoff = 0);

