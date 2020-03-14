
// SDCardWriterDlg.h: 头文件
//

#pragma once


// CSDCardWriterDlg 对话框
class CSDCardWriterDlg : public CDialogEx
{
// 构造
public:
	CSDCardWriterDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SDCARDWRITER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD dwData);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedInputBtn();
private:
	enum Status { STATUS_IDLE = 0, STATUS_READING, STATUS_WRITING, STATUS_VERIFYING, STATUS_EXIT, STATUS_CANCELED };
	CString filePath;
	char deviceLabel;
	char devicePaths[5];
private:
	int get_usb_disk(char usb_paths[]);
	void initCombobox();
	HANDLE getHandleOnFile(LPCWSTR filelocation, DWORD access);
	HANDLE getHandleOnVolume(char volume, DWORD access);
	bool getLockOnVolume(HANDLE handle);
	bool removeLockOnVolume(HANDLE handle);
	bool unmountVolume(HANDLE handle);
	bool isVolumeUnmounted(HANDLE handle);
	DWORD getDeviceID(HANDLE hVolume);
	HANDLE getHandleOnDevice(int device, DWORD access);
	char* readSectorDataFromHandle(HANDLE handle, unsigned long long startsector, unsigned long long numsectors, unsigned long long sectorsize);
	bool writeSectorDataToHandle(HANDLE handle, char* data, unsigned long long startsector, unsigned long long numsectors, unsigned long long sectorsize);
	unsigned long long getFileSizeInSectors(HANDLE handle, unsigned long long sectorsize);
	unsigned long long getNumberOfSectors(HANDLE handle, unsigned long long* sectorsize);
public:
	CComboBox m_cbDevice;
	afx_msg void OnCbnSelchangeDeviceComb();
	afx_msg void OnBnClickedBtnBurn();
	afx_msg void OnStnClickedStaticLink();
};
