// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� PIC2PDF_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// PIC2PDF_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef PIC2PDF_EXPORTS
#define PIC2PDF_API __declspec(dllexport)
#else
#define PIC2PDF_API __declspec(dllimport)
#endif

// �����Ǵ� pic2pdf.dll ������
class PIC2PDF_API Cpic2pdf {
public:
	Cpic2pdf(void);
	// TODO:  �ڴ�������ķ�����
};

extern PIC2PDF_API int npic2pdf;

PIC2PDF_API int fnpic2pdf(void);
