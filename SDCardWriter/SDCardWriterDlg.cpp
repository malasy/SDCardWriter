
// SDCardWriterDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "SDCardWriter.h"
#include "SDCardWriterDlg.h"
#include "afxdialogex.h"
#include <Dbt.h>
#include "winioctl.h"
#include "ioapiset.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SECTION_NAME "SDCardWriter"
#define BLOCK_SIZE_KEY "block_size"
#define BLOCK_COUNT_KEY "block_count"
#define WINDOW_X "left"
#define WINDOW_Y "top"


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSDCardWriterDlg 对话框


CSDCardWriterDlg::CSDCardWriterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SDCARDWRITER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_SD);
}

void CSDCardWriterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICE_COMB, m_cbDevice);
}

BEGIN_MESSAGE_MAP(CSDCardWriterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED(IDC_INPUT_BTN, &CSDCardWriterDlg::OnBnClickedInputBtn)
	ON_CBN_SELCHANGE(IDC_DEVICE_COMB, &CSDCardWriterDlg::OnCbnSelchangeDeviceComb)
	ON_BN_CLICKED(IDC_BTN_BURN, &CSDCardWriterDlg::OnBnClickedBtnBurn)
	ON_STN_CLICKED(IDC_STATIC_LINK, &CSDCardWriterDlg::OnStnClickedStaticLink)
END_MESSAGE_MAP()


// CSDCardWriterDlg 消息处理程序

BOOL CSDCardWriterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	initCombobox();
	
	//读取配置文件
	CString blockSize = AfxGetApp()->GetProfileString(_T(SECTION_NAME), _T(BLOCK_SIZE_KEY));
	CString blockCount = AfxGetApp()->GetProfileString(_T(SECTION_NAME), _T(BLOCK_COUNT_KEY));
	GetDlgItem(IDC_EDIT_BLOCK_SIZE)->SetWindowTextW(blockSize);
	GetDlgItem(IDC_EDIT_OFFSET)->SetWindowTextW(blockCount);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSDCardWriterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSDCardWriterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSDCardWriterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//监听USB设备的插入和移除
BOOL CSDCardWriterDlg::OnDeviceChange(UINT nEventType, DWORD dwData)
{
	if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
		initCombobox();
	}

	if (nEventType == DBT_DEVICEARRIVAL) {
		initCombobox();
	}
	
	return false;
}


//打开资源管理器选择文件
void CSDCardWriterDlg::OnBnClickedInputBtn()
{
	filePath = _T("");
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Bin文件(*.bin)|*.bin|All Files(*.*)|*.*||"), NULL);
	if (dlgFile.DoModal()) {
		filePath = dlgFile.GetPathName();
		if (!filePath.IsEmpty()) {
			GetDlgItem(IDC_INPUT_ET)->SetWindowTextW(filePath);
		}
	}
}

//获取所有的USB设备盘符（一个字母）
int CSDCardWriterDlg::get_usb_disk(char usb_paths[])
{
	DWORD all_disk = GetLogicalDrives();
	int usb_cnt = 0;
	int i = 0;
	char disk_path[5] = { 0 };
	char device_path[10] = { 0 };
	char device_name[100] = {0};
	while (all_disk && usb_cnt < 5) {
		if ((all_disk & 0x1) == 1) {
			sprintf_s(device_path, "%c:\\", 'A' + i);
			if (GetDriveType(CString(device_path)) == DRIVE_REMOVABLE) {
				usb_paths[usb_cnt++] = 'A' + i;
			}
		}
		all_disk = all_disk >> 1;
		i++;
	}
	return usb_cnt;
}

//重新填充下拉列表数据
void CSDCardWriterDlg::initCombobox()
{
	//清空下拉列表
	m_cbDevice.ResetContent();
	int count = get_usb_disk(devicePaths);
	if (count == 0) {
		m_cbDevice.AddString(_T(""));
		m_cbDevice.SetCurSel(0);
		deviceLabel = '#';
		return;
	}
	CString strInfo;
	for (int i = 0; i < count; i++) {
		strInfo.Format(_T("%c:"), devicePaths[i]);
		m_cbDevice.InsertString(i, strInfo);
	}
	//默认选中第一个
	m_cbDevice.SetCurSel(0);
	deviceLabel = devicePaths[0];
}

HANDLE CSDCardWriterDlg::getHandleOnFile(LPCWSTR filelocation, DWORD access)
{
	HANDLE hFile;
	hFile = CreateFileW(filelocation, access, (access == GENERIC_READ) ? FILE_SHARE_READ : 0, NULL, (access == GENERIC_READ) ? OPEN_EXISTING : CREATE_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		wchar_t* errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errInfo;
		errInfo.Format(_T("An error occurred when attempting to get a handle on the file.  Error:%d  %s"), GetLastError(), errormessage);
		MessageBox(errInfo);
		LocalFree(errormessage);
	}
	return hFile;
}

HANDLE CSDCardWriterDlg::getHandleOnVolume(char volume, DWORD access)
{
	HANDLE hVolume;
	char volumename[] = "\\\\.\\A:";
	volumename[4] = volume;
	hVolume = CreateFile(CString(volumename), access, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hVolume == INVALID_HANDLE_VALUE)
	{
		wchar_t* errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errInfo;
		errInfo.Format(_T("An error occurred when attempting to get handle on volume.  Error:%d  %s"), GetLastError(), errormessage);
		MessageBox(errInfo);
	}
	return hVolume;
}

bool CSDCardWriterDlg::getLockOnVolume(HANDLE handle)
{
	DWORD bytesreturned;
	BOOL bResult;
	bResult = DeviceIoControl(handle, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytesreturned, NULL);
	if (!bResult)
	{
		wchar_t* errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errInfo;
		errInfo.Format(_T("An error occurred when attempting to lock the volume.  Error:%d  %s"), GetLastError(), errormessage);
		MessageBox(errInfo);
	}
	return (bResult);
}

bool CSDCardWriterDlg::removeLockOnVolume(HANDLE handle)
{
	DWORD junk;
	BOOL bResult;
	bResult = DeviceIoControl(handle, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &junk, NULL);
	if (!bResult)
	{
		wchar_t* errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errInfo;
		errInfo.Format(_T("An error occurred when attempting to unlock the volume.  Error:%d  %s"), GetLastError(), errormessage);
		MessageBox(errInfo);
	}
	return (bResult);
}

bool CSDCardWriterDlg::unmountVolume(HANDLE handle)
{
	DWORD junk;
	BOOL bResult;
	bResult = DeviceIoControl(handle, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &junk, NULL);
	if (!bResult)
	{
		wchar_t* errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errInfo;
		errInfo.Format(_T("An error occurred when attempting to dismount the volume.  Error:%d  %s"), GetLastError(), errormessage);
		MessageBox(errInfo);
	}
	return (bResult);
}

bool CSDCardWriterDlg::isVolumeUnmounted(HANDLE handle)
{
	DWORD junk;
	BOOL bResult;
	bResult = DeviceIoControl(handle, FSCTL_IS_VOLUME_MOUNTED, NULL, 0, NULL, 0, &junk, NULL);
	return (!bResult);
}

DWORD CSDCardWriterDlg::getDeviceID(HANDLE hVolume)
{
	VOLUME_DISK_EXTENTS sd;
	DWORD bytesreturned;
	if (!DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &sd, sizeof(sd), &bytesreturned, NULL))
	{
		wchar_t* errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errInfo;
		errInfo.Format(_T("An error occurred when attempting to get information on volume.  Error:%d  %s"), GetLastError(), errormessage);
		MessageBox(errInfo);
	}
	return sd.Extents[0].DiskNumber;
}

HANDLE CSDCardWriterDlg::getHandleOnDevice(int device, DWORD access)
{
	HANDLE hDevice;
	CString devicename;
	devicename.Format(_T("\\\\.\\PhysicalDrive%d"), device);

	hDevice = CreateFile(devicename, access, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		wchar_t* errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errInfo;
		errInfo.Format(_T("An error occurred when attempting to get a handle on the device.  Error:%d  %s"), GetLastError(), errormessage);
		MessageBox(errInfo);
		LocalFree(errormessage);
	}
	return hDevice;
}

char* CSDCardWriterDlg::readSectorDataFromHandle(HANDLE handle, unsigned long long startsector, unsigned long long numsectors, unsigned long long sectorsize)
{
	unsigned long bytesread;
	char* data = new char[sectorsize * numsectors];
	LARGE_INTEGER li;
	li.QuadPart = startsector * sectorsize;
	SetFilePointer(handle, li.LowPart, &li.HighPart, FILE_BEGIN);
	if (!ReadFile(handle, data, sectorsize * numsectors, &bytesread, NULL))
	{
		wchar_t* errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errInfo;
		errInfo.Format(_T("An error occurred when attempting to read data from handle.  Error:%d  %s"), GetLastError(), errormessage);
		MessageBox(errInfo);
		delete[] data;
		data = NULL;
	}
	if (data && bytesread < (sectorsize * numsectors))
	{
		memset(data + bytesread, 0, (sectorsize * numsectors) - bytesread);
	}
	return data;
}

bool CSDCardWriterDlg::writeSectorDataToHandle(HANDLE handle, char* data, unsigned long long startsector, unsigned long long numsectors, unsigned long long sectorsize)
{
	unsigned long byteswritten;
	BOOL bResult;
	LARGE_INTEGER li;
	li.QuadPart = startsector * sectorsize;
	SetFilePointer(handle, li.LowPart, &li.HighPart, FILE_BEGIN);
	bResult = WriteFile(handle, data, sectorsize * numsectors, &byteswritten, NULL);
	if (!bResult)
	{
		wchar_t* errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errInfo;
		errInfo.Format(_T("An error occurred when attempting to write data to handle.  Error:%d  %s"), GetLastError(), errormessage);
		MessageBox(errInfo);
	}
	return (bResult);
}

unsigned long long CSDCardWriterDlg::getFileSizeInSectors(HANDLE handle, unsigned long long sectorsize)
{
	unsigned long long retVal = 0;
	if (sectorsize) // avoid divide by 0
	{
		LARGE_INTEGER filesize;
		if (GetFileSizeEx(handle, &filesize) == 0)
		{
			wchar_t* errormessage = NULL;
			FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
			CString errInfo;
			errInfo.Format(_T("An error occurred while getting the file size.  Error:%d  %s"), GetLastError(), errormessage);
			MessageBox(errInfo);
			retVal = 0;
		}
		else
		{
			retVal = ((unsigned long long)filesize.QuadPart / sectorsize) + (((unsigned long long)filesize.QuadPart % sectorsize) ? 1 : 0);
		}
	}
	return(retVal);
}

unsigned long long CSDCardWriterDlg::getNumberOfSectors(HANDLE handle, unsigned long long* sectorsize)
{
	DWORD junk;
	DISK_GEOMETRY_EX diskgeometry;
	BOOL bResult;
	bResult = DeviceIoControl(handle, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskgeometry, sizeof(diskgeometry), &junk, NULL);
	if (!bResult)
	{
		wchar_t* errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errInfo;
		errInfo.Format(_T("An error occurred when attempting to get the device's geometry.  Error:%d  %s"), GetLastError(), errormessage);
		MessageBox(errInfo);
		return 0;
	}
	if (sectorsize != NULL)
	{
		*sectorsize = (unsigned long long)diskgeometry.Geometry.BytesPerSector;
	}
	return (unsigned long long)diskgeometry.DiskSize.QuadPart / (unsigned long long)diskgeometry.Geometry.BytesPerSector;
}

//下拉选中列表中的另外一项
void CSDCardWriterDlg::OnCbnSelchangeDeviceComb()
{
	int sel = m_cbDevice.GetCurSel();
	deviceLabel = devicePaths[sel];
}


//点击烧录按钮
void CSDCardWriterDlg::OnBnClickedBtnBurn()
{
	//获取必须数据：烧录的文件，烧录的设备，偏移，块大小
	if (filePath.IsEmpty() || deviceLabel == '#') {
		MessageBox(_T("请选择设备和烧录的文件"));
		return;
	}
	CString blockSizeStr;
	CString blockCountStr;
	GetDlgItem(IDC_EDIT_BLOCK_SIZE)->GetWindowTextW(blockSizeStr);
	GetDlgItem(IDC_EDIT_OFFSET)->GetWindowTextW(blockCountStr);
	if (blockSizeStr.IsEmpty() || blockCountStr.IsEmpty()) {
		MessageBox(_T("请输入块大小和偏移量"));
		return;
	}
	//保存数据
	AfxGetApp()->WriteProfileStringW(_T(SECTION_NAME), _T(BLOCK_SIZE_KEY), blockSizeStr);
	AfxGetApp()->WriteProfileStringW(_T(SECTION_NAME), _T(BLOCK_COUNT_KEY), blockCountStr);

	//字符串-->数字
	unsigned long long blockSize = _ttoi(blockSizeStr);
	unsigned int blockCount = _ttoi(blockCountStr);

	int status = STATUS_WRITING;
	//获取卷句柄
	HANDLE hVolume = getHandleOnVolume(deviceLabel, GENERIC_WRITE);
	if (hVolume == INVALID_HANDLE_VALUE) { 
		status = STATUS_IDLE;
		return; 
	}
	//获取设备ID
	DWORD deviceID = getDeviceID(hVolume);
	if (!getLockOnVolume(hVolume))
	{
		CloseHandle(hVolume);
		hVolume = INVALID_HANDLE_VALUE;
		status = STATUS_IDLE;
		return;
	}
	//卸载卷（不懂为什么要这样，猜测应该是需要卸载设备，解除系统对该设备的占用，然后我们才能直接操作该设备）
	if (!unmountVolume(hVolume))
	{
		removeLockOnVolume(hVolume);
		CloseHandle(hVolume);
		hVolume = INVALID_HANDLE_VALUE;
		status = STATUS_IDLE;
		return;
	}

	//获取文件句柄
	HANDLE hFile = getHandleOnFile(filePath, GENERIC_READ);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		removeLockOnVolume(hVolume);
		CloseHandle(hVolume);
		hVolume = INVALID_HANDLE_VALUE;
		status = STATUS_IDLE;
		return;
	}

	//得到设备的物理地址。hVolume是应用层使用的地址，hRawDisk是驱动层使用的地址（不太确定）
	HANDLE hRawDisk = getHandleOnDevice(deviceID, GENERIC_WRITE);
	if (hRawDisk == INVALID_HANDLE_VALUE)
	{
		removeLockOnVolume(hVolume);
		CloseHandle(hFile);
		CloseHandle(hVolume);
		hVolume = INVALID_HANDLE_VALUE;
		hFile = INVALID_HANDLE_VALUE;
		status = STATUS_IDLE;
		return;
	}
	unsigned long long numsectors,sectorsize, availablesectors, lasti,i;

	//获取可用空间(以扇区为单位)和扇区大小，一般扇区大小为512Byte
	availablesectors = getNumberOfSectors(hRawDisk, &sectorsize);
	if (!availablesectors) { 
		removeLockOnVolume(hVolume);
		CloseHandle(hRawDisk);
		CloseHandle(hFile);
		CloseHandle(hVolume);
		hRawDisk = INVALID_HANDLE_VALUE;
		hFile = INVALID_HANDLE_VALUE;
		hVolume = INVALID_HANDLE_VALUE;
		status = STATUS_IDLE;
		return; 
	}

	//获取文件大小（以扇区为单位）
	numsectors = getFileSizeInSectors(hFile, sectorsize);
	if (!numsectors)
	{
		removeLockOnVolume(hVolume);
		CloseHandle(hRawDisk);
		CloseHandle(hFile);
		CloseHandle(hVolume);
		hRawDisk = INVALID_HANDLE_VALUE;
		hFile = INVALID_HANDLE_VALUE;
		hVolume = INVALID_HANDLE_VALUE;
		status = STATUS_IDLE;
		return;
	}

	if (numsectors > availablesectors) {
		MessageBox(_T("More space required than is available"));
	}
	lasti = 0ul;
	char* sectorData;

	//计算需要偏移的扇区数
	unsigned long long offsetSector = (blockSize * blockCount)/sectorsize;
	for (i = 0ul; i < numsectors && status == STATUS_WRITING; i += 1024ul) {
		sectorData = readSectorDataFromHandle(hFile, i, (numsectors - i >= 1024ul) ? 1024ul : (numsectors - i), sectorsize);
		if (sectorData == NULL)
		{
			removeLockOnVolume(hVolume);
			CloseHandle(hRawDisk);
			CloseHandle(hFile);
			CloseHandle(hVolume);
			status = STATUS_IDLE;
			hRawDisk = INVALID_HANDLE_VALUE;
			hFile = INVALID_HANDLE_VALUE;
			hVolume = INVALID_HANDLE_VALUE;
			return;
		}
		if (!writeSectorDataToHandle(hRawDisk, sectorData, offsetSector + i, (numsectors - i >= 1024ul) ? 1024ul : (numsectors - i), sectorsize))
		{
			delete[] sectorData;
			removeLockOnVolume(hVolume);
			CloseHandle(hRawDisk);
			CloseHandle(hFile);
			CloseHandle(hVolume);
			status = STATUS_IDLE;
			sectorData = NULL;
			hRawDisk = INVALID_HANDLE_VALUE;
			hFile = INVALID_HANDLE_VALUE;
			hVolume = INVALID_HANDLE_VALUE;
			return;
		}
		delete[] sectorData;
		sectorData = NULL;
	}
	removeLockOnVolume(hVolume);
	CloseHandle(hRawDisk);
	CloseHandle(hFile);
	CloseHandle(hVolume);
	hRawDisk = INVALID_HANDLE_VALUE;
	hFile = INVALID_HANDLE_VALUE;
	hVolume = INVALID_HANDLE_VALUE;
	MessageBox(_T("烧录成功!"));
}


void CSDCardWriterDlg::OnStnClickedStaticLink()
{
	ShellExecute(0, NULL, _T("https://github.com/malasy/SDCardWriter"), NULL, NULL, SW_NORMAL);
}
