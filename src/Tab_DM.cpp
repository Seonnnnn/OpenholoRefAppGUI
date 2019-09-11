// Tab_DM.cpp : implementation file
//

#include "stdafx.h"
#include "OpenholoRefAppGUI.h"
#include "Tab_DM.h"
#include "afxdialogex.h"

#include <ophDepthMap.h>
#include "Dialog_BMP_Viewer.h"
#include "Dialog_Progress.h"
#include "Dialog_Prompt.h"

// CTab_DM dialog

IMPLEMENT_DYNAMIC(CTab_DM, CDialogEx)

CTab_DM::CTab_DM(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_DM, pParent)
	, m_fieldLens(0)
	, m_nearDepth(0)
	, m_farDepth(0)
	, m_numDepth(0)
	, m_pixelpitchX(0)
	, m_pixelpitchY(0)
	, m_pixelnumX(0)
	, m_pixelnumY(0)
	, m_wavelength(0)
	, m_bConfig(false)
	, m_bDimg(false)
	, m_bRGBimg(false)
	, m_bEncode(false)
	, m_argParamDimg()
	, m_argParamRGBimg()
	, m_resultPath()
	, m_idxEncode(6)
#ifdef TEST_MODE
	, m_bTest(FALSE)
#endif
{

}

CTab_DM::~CTab_DM()
{
}

void CTab_DM::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FIELD_LENS, m_fieldLens);
	DDX_Text(pDX, IDC_NEAR_DEPTH, m_nearDepth);
	DDX_Text(pDX, IDC_FAR_DEPTH, m_farDepth);
	DDX_Text(pDX, IDC_NUM_OF_DEPTH, m_numDepth);
	DDX_Text(pDX, IDC_PIXEL_PITCH_X, m_pixelpitchX);
	DDX_Text(pDX, IDC_PIXEL_PITCH_Y, m_pixelpitchY);
	DDX_Text(pDX, IDC_PIXEL_NUM_X, m_pixelnumX);
	DDX_Text(pDX, IDC_PIXEL_NUM_Y, m_pixelnumY);
	DDX_Text(pDX, IDC_WAVE_LENGTH, m_wavelength);
	DDX_Control(pDX, IDC_GPU_CHECK_DM, m_buttonGPU);
	DDX_Control(pDX, IDC_SAVE_BMP_DM, m_buttonSaveBmp);
	DDX_Control(pDX, IDC_SAVE_OHC_DM, m_buttonSaveOhc);
	DDX_Control(pDX, IDC_TRANSFORM_VW, m_buttonViewingWindow);
}


BEGIN_MESSAGE_MAP(CTab_DM, CDialogEx)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_READ_CONFIG_DM, &CTab_DM::OnBnClickedReadConfig_DM)
	ON_BN_CLICKED(IDC_LOAD_D_IMG, &CTab_DM::OnBnClickedLoadDImg)
	ON_BN_CLICKED(IDC_LOAD_RGB_IMG, &CTab_DM::OnBnClickedLoadRgbImg)
	ON_BN_CLICKED(IDC_GENERATE_DM, &CTab_DM::OnBnClickedGenerate_DM)
	ON_BN_CLICKED(IDC_SAVE_BMP_DM, &CTab_DM::OnBnClickedSaveBmp_DM)
	ON_BN_CLICKED(IDC_SAVE_OHC_DM, &CTab_DM::OnBnClickedSaveOhc_DM)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_VIEW_DM, &CTab_DM::OnBnClickedViewDm)
	ON_BN_CLICKED(IDC_VIEW_DM_BMP, &CTab_DM::OnBnClickedViewDmBmp)
	ON_BN_CLICKED(IDC_VIEW_DM_IMG, &CTab_DM::OnBnClickedViewDmImg)
	ON_CBN_SELCHANGE(IDC_ENCODE_METHOD_DM, &CTab_DM::OnCbnSelchangeEncodeMethodDm)
	ON_BN_CLICKED(IDC_ENCODING_DM, &CTab_DM::OnBnClickedEncodingDm)
END_MESSAGE_MAP()


// CTab_DM message handlers
BOOL CTab_DM::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class	
	if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) return TRUE;
#ifdef TEST_MODE
	if (!m_bTest && pMsg->wParam == VK_SPACE) {
		AutoTest();
		return TRUE;
	}
#endif
	return CDialogEx::PreTranslateMessage(pMsg);
}


#ifdef TEST_MODE
BOOL CTab_DM::AutoTest()
{
	if (!m_bDimg || !m_bRGBimg || !m_bConfig)
		return FALSE;
	m_bTest = TRUE;
	Dialog_Prompt *prompt = new Dialog_Prompt;
	if (IDOK == prompt->DoModal()) {
		int nRepeat = prompt->GetInputInteger();
		for (int i = 0; i < nRepeat; i++)
			SendMessage(WM_COMMAND, MAKEWPARAM(IDC_GENERATE_DM, BN_CLICKED), 0L);
	}
	delete prompt;
	m_bTest = FALSE;
	return TRUE;
}
#endif

int CTab_DM::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_pDepthMap = new ophDepthMap();

	return 0;
}


void CTab_DM::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	//SetWindowPos(NULL, 0, 0, 353, 305, SWP_NOMOVE);
}


void CTab_DM::OnBnClickedReadConfig_DM()
{
	// TODO: Add your control notification handler code here	
	TCHAR current_path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, current_path);

	LPTSTR szFilter = L"XML File (*.xml) |*.xml|";

	CFileDialog FileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	CString path;
	if (FileDialog.DoModal() == IDOK)
	{
		CString ext = FileDialog.GetFileExt();
		if (ext == "xml") path = FileDialog.GetFolderPath() + L"\\" + FileDialog.GetFileName();
		else return;
	}

	SetCurrentDirectory(current_path);

	TCHAR widepath[MAX_PATH] = { 0 };
	char mulpath[MAX_PATH] = { 0 };

	_tcscpy_s(widepath, path.GetBuffer());
	WideCharToMultiByte(CP_ACP, 0, widepath, MAX_PATH, mulpath, MAX_PATH, NULL, NULL);
	if (strcmp(mulpath, "") == 0) return;

	if (!m_pDepthMap->readConfig(mulpath)) {
		AfxMessageBox(TEXT("it is not xml config file for DepthMap."));
		return;
	}

	auto context = m_pDepthMap->getContext();
	//auto config = m_pDepthMap->getConfig();

	m_fieldLens = m_pDepthMap->getFieldLens();
	m_nearDepth = m_pDepthMap->getNearDepth();
	m_farDepth = m_pDepthMap->getFarDepth();
	m_numDepth = m_pDepthMap->getNumOfDepth();
	m_pixelpitchX = context.pixel_pitch[_X];
	m_pixelpitchY = context.pixel_pitch[_Y];
	int pixX = context.pixel_number[_X];
	m_pixelnumX = context.pixel_number[_X];//pixX / 3;
	m_pixelnumY = context.pixel_number[_Y];
	m_wavelength = *context.wave_length;

	m_bConfig = true;
	//if (m_bDimg && m_bRGBimg) GetDlgItem(IDC_GENERATE_DM)->EnableWindow(TRUE);
	GetDlgItem(IDC_LOAD_D_IMG)->EnableWindow(TRUE);
	GetDlgItem(IDC_LOAD_RGB_IMG)->EnableWindow(TRUE);

	UpdateData(FALSE);
}


void CTab_DM::OnBnClickedLoadDImg()
{
	// TODO: Add your control notification handler code here	
	TCHAR current_path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, current_path);

	LPTSTR szFilter = L"BMP File (*.bmp) |*.bmp|";

	CFileDialog FileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	CString path;
	if (FileDialog.DoModal() == IDOK)
	{
		CString ext = FileDialog.GetFileExt();
		if (ext == "bmp") {
			path = FileDialog.GetFolderPath() + L"\\" + FileDialog.GetFileName();
			m_szPath = FileDialog.GetFolderPath();
			m_szDname = FileDialog.GetFileTitle();
		}
		else return;
	}

	SetCurrentDirectory(current_path);

	TCHAR widepath[MAX_PATH] = { 0 };
	char mulpath[MAX_PATH] = { 0 };

	_tcscpy_s(widepath, path.GetBuffer());
	_tcscpy_s(m_argParamDimg, path.GetBuffer());
	WideCharToMultiByte(CP_ACP, 0, widepath, MAX_PATH, mulpath, MAX_PATH, NULL, NULL);
	if (strcmp(mulpath, "") == 0) return;


	m_bDimg = TRUE;
	if (m_bRGBimg) {
		_tcscpy_s(widepath, m_szPath.GetBuffer());
		WideCharToMultiByte(CP_ACP, 0, widepath, MAX_PATH, mulpath, MAX_PATH, NULL, NULL);

		TCHAR wideDname[MAX_PATH] = { 0 };
		char mulDname[MAX_PATH] = { 0 };
		_tcscpy_s(wideDname, m_szDname.GetBuffer());
		WideCharToMultiByte(CP_ACP, 0, wideDname, MAX_PATH, mulDname, MAX_PATH, NULL, NULL);

		TCHAR wideRGBname[MAX_PATH] = { 0 };
		char mulRGBname[MAX_PATH] = { 0 };
		_tcscpy_s(wideRGBname, m_szRGBname.GetBuffer());
		WideCharToMultiByte(CP_ACP, 0, wideRGBname, MAX_PATH, mulRGBname, MAX_PATH, NULL, NULL);

		if (!m_pDepthMap->readImageDepth(mulpath, mulRGBname, mulDname)) {
			AfxMessageBox(_TEXT("BMP load failed : Please show LOG."));
		}

		GetDlgItem(IDC_VIEW_DM_IMG)->EnableWindow(TRUE);
		GetDlgItem(IDC_VIEW_DM_MODEL)->EnableWindow(TRUE);
		GetDlgItem(IDC_GENERATE_DM)->EnableWindow(TRUE);
	}
}


void CTab_DM::OnBnClickedLoadRgbImg()
{
	// TODO: Add your control notification handler code here
	TCHAR current_path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, current_path);

	LPTSTR szFilter = L"BMP File (*.bmp) |*.bmp|";

	CFileDialog FileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	CString path;
	if (FileDialog.DoModal() == IDOK)
	{
		CString ext = FileDialog.GetFileExt();
		if (ext == "bmp") {
			path = FileDialog.GetFolderPath() + L"\\" + FileDialog.GetFileName();
			m_szPath = FileDialog.GetFolderPath();
			m_szRGBname = FileDialog.GetFileTitle();
		}
		else return;
	}

	SetCurrentDirectory(current_path);

	TCHAR widepath[MAX_PATH] = { 0 };
	char mulpath[MAX_PATH] = { 0 };

	_tcscpy_s(widepath, path.GetBuffer());
	_tcscpy_s(m_argParamRGBimg, path.GetBuffer());
	WideCharToMultiByte(CP_ACP, 0, widepath, MAX_PATH, mulpath, MAX_PATH, NULL, NULL);
	if (strcmp(mulpath, "") == 0) return;

	m_bRGBimg = TRUE;
	if (m_bDimg) {
		_tcscpy_s(widepath, m_szPath.GetBuffer());
		WideCharToMultiByte(CP_ACP, 0, widepath, MAX_PATH, mulpath, MAX_PATH, NULL, NULL);

		TCHAR wideDname[MAX_PATH] = { 0 };
		char mulDname[MAX_PATH] = { 0 };
		_tcscpy_s(wideDname, m_szDname.GetBuffer());
		WideCharToMultiByte(CP_ACP, 0, wideDname, MAX_PATH, mulDname, MAX_PATH, NULL, NULL);

		TCHAR wideRGBname[MAX_PATH] = { 0 };
		char mulRGBname[MAX_PATH] = { 0 };
		_tcscpy_s(wideRGBname, m_szRGBname.GetBuffer());
		WideCharToMultiByte(CP_ACP, 0, wideRGBname, MAX_PATH, mulRGBname, MAX_PATH, NULL, NULL);


		if (!m_pDepthMap->readImageDepth(mulpath, mulRGBname, mulDname)) {
			AfxMessageBox(_TEXT("BMP load failed : Please show LOG."));
		}

		GetDlgItem(IDC_VIEW_DM_IMG)->EnableWindow(TRUE);
		GetDlgItem(IDC_VIEW_DM_MODEL)->EnableWindow(TRUE);
		GetDlgItem(IDC_GENERATE_DM)->EnableWindow(TRUE);
	}
}

void CTab_DM::OnBnClickedViewDmImg()
{
	// TODO: Add your control notification handler code here
	Dialog_BMP_Viewer viewer;

	std::vector<CString> imagePathArr;
	imagePathArr.push_back(m_argParamDimg);
	imagePathArr.push_back(m_argParamRGBimg);

	viewer.Init(imagePathArr, INIT_ARR);
	viewer.DoModal();

	viewer.DestroyWindow();
}

void CTab_DM::OnBnClickedViewDm()
{
	// TODO: Add your control notification handler code here
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, sizeof(path));

	CString localPath = path;
	int i = localPath.ReverseFind('\\');
	localPath = localPath.Left(i);
	localPath.Append(L"\\3D_Object_Viewer.exe");
	_tcscpy_s(path, localPath.GetBuffer());

	TCHAR argParam[MAX_PATH * 3] = { 0 };

	CString szArgParamD = CString("\"") + (m_argParamDimg)+CString("\"");
	CString szArgParamRGB = CString("\"") + (m_argParamRGBimg)+CString("\"");

	wsprintf(argParam, L"%d %s %s", 1, szArgParamD.GetBuffer(), szArgParamRGB.GetBuffer());

	auto a = (int)::ShellExecute(NULL, _T("open"),
		path,																								//���� ���� ���
		argParam,																							//argument value �Ķ����
		NULL, SW_SHOW);
}

UINT CallFuncDM(void* param)
{
	((ophDepthMap*)((parammeter*)param)->pGEN)->generateHologram();

	*((parammeter*)param)->pFinish = TRUE;
	((parammeter*)param)->pDialog->m_bFinished = TRUE;
	//::SendMessage(((parammeter*)param)->pDialog->GetSafeHwnd(), WM_CLOSE, NULL, NULL);
	delete param;

	return 1;
}

void CTab_DM::OnBnClickedGenerate_DM()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	if (m_fieldLens == 0.0) {
		AfxMessageBox(TEXT("Config value error - field lens"));
		return;
	}
	if (m_numDepth == 0) {
		AfxMessageBox(TEXT("Config value error - number of depth"));
		return;
	}
	if (m_pixelpitchX == 0.0 || m_pixelpitchY == 0.0) {
		AfxMessageBox(TEXT("Config value error - pixel pitch"));
		return;
	}
	if (m_pixelnumX == 0 || m_pixelnumY == 0) {
		AfxMessageBox(TEXT("Config value error - pixel number"));
		return;
	}
	if (m_wavelength == 0.0) {
		AfxMessageBox(TEXT("Config value error - wave length"));
		return;
	}

	auto context = m_pDepthMap->getContext();
	auto config = m_pDepthMap->getConfig();

	context.pixel_number = ivec2(m_pixelnumX * 3, m_pixelnumY);
	context.pixel_pitch = vec2(m_pixelpitchX, m_pixelpitchY);
	context.wave_length[0] = m_wavelength;

	config.DEFAULT_DEPTH_QUANTIZATION = m_numDepth;
	config.far_depthmap = m_farDepth;
	config.near_depthmap = m_nearDepth;
	config.field_lens = m_fieldLens;
	config.FLAG_CHANGE_DEPTH_QUANTIZATION = 1;
	config.NUMBER_OF_DEPTH_QUANTIZATION = m_numDepth;
	config.num_of_depth = m_numDepth;
	config.RANDOM_PHASE = 0;

	m_pDepthMap->setMode(!m_buttonGPU.GetCheck());
	m_pDepthMap->setViewingWindow(m_buttonViewingWindow.GetCheck());

	Dialog_Progress progress;

	BOOL bIsFinish = FALSE;

	parammeter *pParam = new parammeter;
	pParam->pGEN = m_pDepthMap;
	pParam->pDialog = &progress;
	pParam->pFinish = &bIsFinish;

	CWinThread* pThread = AfxBeginThread(CallFuncDM, pParam);
	progress.DoModal();
	progress.DestroyWindow();

	//m_pDepthMap->generateHologram();
	GetDlgItem(IDC_SAVE_OHC_DM)->EnableWindow(TRUE);
	GetDlgItem(IDC_SAVE_BMP_DM)->EnableWindow(FALSE);
	GetDlgItem(IDC_ENCODING_DM)->EnableWindow(TRUE);

	UpdateData(FALSE);
}

void CTab_DM::OnBnClickedEncodingDm()
{
	// TODO: Add your control notification handler code here
	if (m_idxEncode == ophGen::ENCODE_SSB) {
		m_pDepthMap->encodeHologram();
		m_pDepthMap->normalize();
	}
	else {
		//m_pDepthMap->waveCarry(0, 0.5);
		switch (ophGen::ENCODE_FLAG(m_idxEncode)) {
		case ophGen::ENCODE_PHASE:
		case ophGen::ENCODE_AMPLITUDE:
		case ophGen::ENCODE_REAL:
		case ophGen::ENCODE_SIMPLENI:
		case ophGen::ENCODE_BURCKHARDT:
		case ophGen::ENCODE_TWOPHASE:
			m_pDepthMap->encoding(ophGen::ENCODE_FLAG(m_idxEncode));
			break;
		case ophGen::ENCODE_SSB:
			m_pDepthMap->encoding(ophGen::ENCODE_FLAG(m_idxEncode), ophGen::SSB_TOP);
			break;
		case ophGen::ENCODE_OFFSSB:
			m_pDepthMap->encoding(ophGen::ENCODE_FLAG(m_idxEncode), ophGen::SSB_TOP);
			break;
		}
		m_pDepthMap->normalizeEncoded();
	}

	m_buttonSaveBmp.EnableWindow(TRUE);
}

void CTab_DM::OnBnClickedSaveBmp_DM()
{
	// TODO: Add your control notification handler code here
	TCHAR current_path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, current_path);

	LPTSTR szFilter = L"BMP File (*.bmp) |*.bmp|";

	CFileDialog FileDialog(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	CString path;
	if (FileDialog.DoModal() == IDOK)
	{
		CString ext = FileDialog.GetFileExt();
		if (ext == "bmp") path = FileDialog.GetFolderPath() + L"\\" + FileDialog.GetFileName();
		else path = FileDialog.GetFolderPath() + L"\\" + FileDialog.GetFileName() + L".bmp";
	}

	SetCurrentDirectory(current_path);

	TCHAR widepath[MAX_PATH] = { 0 };
	char mulpath[MAX_PATH] = { 0 };

	_tcscpy_s(widepath, path.GetBuffer());
	_tcscpy_s(m_resultPath, path.GetBuffer());
	WideCharToMultiByte(CP_ACP, 0, widepath, MAX_PATH, mulpath, MAX_PATH, NULL, NULL);

	if (strcmp(mulpath, "") == 0) return;
	m_pDepthMap->save(mulpath, 8);

	GetDlgItem(IDC_VIEW_DM_BMP)->EnableWindow(TRUE);
}

void CTab_DM::OnBnClickedViewDmBmp()
{
	// TODO: Add your control notification handler code here
	Dialog_BMP_Viewer viewer;

	viewer.Init(m_resultPath, INIT_SINGLE);
	viewer.DoModal();

	viewer.DestroyWindow();
}


void CTab_DM::OnBnClickedSaveOhc_DM()
{
	// TODO: Add your control notification handler code here
	TCHAR current_path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, current_path);

	LPTSTR szFilter = L"OHC File (*.ohc) |*.ohc|";

	CFileDialog FileDialog(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	CString path;
	if (FileDialog.DoModal() == IDOK)
	{
		CString ext = FileDialog.GetFileExt();
		if (ext == "ohc") path = FileDialog.GetFolderPath() + L"\\" + FileDialog.GetFileName();
		else path = FileDialog.GetFolderPath() + L"\\" + FileDialog.GetFileName() + L".ohc";
	}

	SetCurrentDirectory(current_path);

	TCHAR widepath[MAX_PATH] = { 0 };
	char mulpath[MAX_PATH] = { 0 };

	_tcscpy_s(widepath, path.GetBuffer());
	WideCharToMultiByte(CP_ACP, 0, widepath, MAX_PATH, mulpath, MAX_PATH, NULL, NULL);

	if (strcmp(mulpath, "") == 0) return;
	if (m_pDepthMap->saveAsOhc(mulpath)) {

		//TCHAR strExecutable[FILENAME_MAX];
		//int result = (int)FindExecutable(widepath, NULL, (LPTSTR)&strExecutable);

		//if (result == 31) {
		//	SHELLEXECUTEINFO sei = { sizeof(sei), 0, m_hWnd, L"Openas",	widepath, NULL, NULL, SW_SHOWNORMAL, AfxGetApp()->m_hInstance };
		//	ShellExecuteEx(&sei);
		//}
		//else if (result == 32) {
		//	SHELLEXECUTEINFO sei = { sizeof(sei), 0, m_hWnd, L"Open", widepath, NULL, NULL,	SW_SHOWNORMAL, AfxGetApp()->m_hInstance };
		//	ShellExecuteEx(&sei);
		//}

		(int)::ShellExecute(NULL, _T("open"),
			widepath,																								//���� ���� ���
			NULL,																							//argument value �Ķ����
			NULL, SW_SHOW);
	}
}


BOOL CTab_DM::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	((CComboBox*)GetDlgItem(IDC_ENCODE_METHOD_DM))->AddString(L"Phase");
	((CComboBox*)GetDlgItem(IDC_ENCODE_METHOD_DM))->AddString(L"Amplitude");
	((CComboBox*)GetDlgItem(IDC_ENCODE_METHOD_DM))->AddString(L"Real");
	((CComboBox*)GetDlgItem(IDC_ENCODE_METHOD_DM))->AddString(L"Simple NI");
	((CComboBox*)GetDlgItem(IDC_ENCODE_METHOD_DM))->AddString(L"Burckhardt");
	((CComboBox*)GetDlgItem(IDC_ENCODE_METHOD_DM))->AddString(L"Two-Phase");
	((CComboBox*)GetDlgItem(IDC_ENCODE_METHOD_DM))->AddString(L"Single-Side Band");
	((CComboBox*)GetDlgItem(IDC_ENCODE_METHOD_DM))->AddString(L"Off-SSB");

	((CComboBox*)GetDlgItem(IDC_ENCODE_METHOD_DM))->SetCurSel(m_idxEncode);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CTab_DM::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	m_pDepthMap->release();
}

void CTab_DM::OnCbnSelchangeEncodeMethodDm()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_idxEncode = ((CComboBox*)GetDlgItem(IDC_ENCODE_METHOD_DM))->GetCurSel();
}