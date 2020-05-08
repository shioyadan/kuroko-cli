#define _UNICODE
#define UNICODE

#pragma comment(lib, "winspool.lib")
#include <windows.h>
#include <winspool.h>
#include <string>
#include <vector>

using namespace std;

string WStringToString(const wstring& str)
{
    const size_t length = str.size()*4+1;
    char *buf = new char[length];
    WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, buf, length, NULL, NULL);
    string ret(buf);
    delete[] buf;
    return ret;
}

wstring StringToWString(const string& str)
{
    wchar_t *buf = new wchar_t[str.size()+1];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buf, str.size()+1);
    wstring ret(buf);
    delete[] buf;
    return ret;
}

template <typename ... T>
string Format(const string& fmt, T ... args)
{
    size_t length = snprintf(nullptr, 0, fmt.c_str(), args ...);
    vector<char> buf(length + 1);
    snprintf(&buf[0], length + 1, fmt.c_str(), args ...);
    return string(&buf[0], &buf[length]);
}

bool LoadBinary(vector<BYTE> *dat, const char* file_name)
{
    FILE *file = fopen(file_name, "rb");
    if(!file)
        return false;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    dat->resize(size);
    fseek(file, 0, SEEK_SET);
    for(size_t i = 0; i < size; ){
        size_t read_size = fread(&((*dat)[i]), 1, 256, file);
        i += read_size;
    }
    fclose(file);
    return true;
};

bool SaveBinary(const vector<BYTE>& dat, const char* file_name)
{
    FILE *file = fopen(file_name, "wb");
    if(!file)
        return false;

    fwrite(&dat[0], 1, dat.size(), file);
    fclose(file);
    return true;
};


int main(int argc, const char* argv[]) {

    wchar_t PRINTER_NAME[] = L"kuroko-printer";
    //wchar_t PRINTER_NAME[] = L"Microsoft Print to PDF";

    wchar_t input_file_name[] = L"mpki.emf";


    // Open kuroko-printer
    HANDLE h_printer = 0;
    PRINTER_DEFAULTS pdef;
    memset(&pdef, 0, sizeof(PRINTER_DEFAULTS));
    pdef.DesiredAccess = PRINTER_ALL_ACCESS;
    OpenPrinter(PRINTER_NAME, &h_printer, &pdef);
    if(h_printer == NULL) {
        wprintf(L"Could not open the \"%s\" printer (%d). \n", PRINTER_NAME, GetLastError());
        return 1;
    }

    // Get EMF header information
    HENHMETAFILE h_emf = GetEnhMetaFile(input_file_name);
    ENHMETAHEADER emf_header;
    memset(&emf_header, 0, sizeof(ENHMETAHEADER));
    GetEnhMetaFileHeader(h_emf, sizeof(ENHMETAHEADER), &emf_header);

    // Get printer configurations
    DWORD printer_info_size_needed;
    GetPrinter(h_printer, 2, 0, 0, &printer_info_size_needed);
    DWORD printer_info_size = printer_info_size_needed;

    HGLOBAL h_mem = GlobalAlloc(GHND, printer_info_size);    
    PRINTER_INFO_2* printer_info = (PRINTER_INFO_2*)GlobalLock(h_mem);
    if (GetPrinter(h_printer, 2, (LPBYTE)printer_info, printer_info_size, &printer_info_size_needed) == 0) {
        printf("Failed GetPrinter()\n");
        return 1;
    }

    // Set A3 size to the printer 
    // A3 paper is the biggest one supported in MS PDF Writer
    printer_info->pDevMode->dmFields = DM_PAPERSIZE;
    printer_info->pDevMode->dmPaperSize = DMPAPER_A3;
    // printer_info->pDevMode->dmPaperWidth = emf_header.rclBounds.right - emf_header.rclBounds.left;
    // printer_info->pDevMode->dmPaperLength = emf_header.rclBounds.bottom - emf_header.rclBounds.top;
    // printer_info->pSecurityDescriptor = NULL;

    if (DocumentProperties(
            NULL, h_printer, PRINTER_NAME, 
            (PDEVMODE)printer_info->pDevMode,
            (PDEVMODE)printer_info->pDevMode,
            DM_IN_BUFFER | DM_OUT_BUFFER/* | DM_IN_PROMPT*/
        ) < 0
    ) {
        printf("Failed DocumentProperties(): (%d)\n", GetLastError());
        return 1;
    }
    if (SetPrinter(h_printer, 2, (LPBYTE)printer_info, 0) == 0) {
        printf("Failed SetPrinter(): (%d)\n", GetLastError());
        return 1;
    }


    // Calculate sizes
    HDC hdc_printer = CreateDC(NULL, PRINTER_NAME, NULL, NULL);
    int x_pixels_per_inch = GetDeviceCaps(hdc_printer, LOGPIXELSX);
    int y_pixels_per_inch = GetDeviceCaps(hdc_printer, LOGPIXELSY);
    int offset_x = GetDeviceCaps(hdc_printer, PHYSICALOFFSETX);
    int offset_y = GetDeviceCaps(hdc_printer, PHYSICALOFFSETY);
    int margins_x = GetDeviceCaps(hdc_printer, PHYSICALWIDTH) - GetDeviceCaps(hdc_printer, HORZRES);
    int margins_y = GetDeviceCaps(hdc_printer, PHYSICALHEIGHT) - GetDeviceCaps(hdc_printer, VERTRES);

    // a unit of rclFrame is 1/100 mm?
    double inchi_size_emf_x = (double)(emf_header.rclFrame.right - emf_header.rclFrame.left) / 100 / 10 / 2.54;
    double inchi_size_emf_y = (double)(emf_header.rclFrame.bottom - emf_header.rclFrame.top) / 100 / 10 / 2.54;
    int pixel_size_emf_x = (int)(inchi_size_emf_x * x_pixels_per_inch);
    int pixel_size_emf_y = (int)(inchi_size_emf_y * y_pixels_per_inch);
    // a unit of dmPaperWidth/dmPaperLength is 1/10 mm
    int pixel_size_page_x = (int)((double)printer_info->pDevMode->dmPaperWidth * 10 / 1000 / 2.54 * x_pixels_per_inch);
    int pixel_size_page_y = (int)((double)printer_info->pDevMode->dmPaperLength * 10 / 1000 / 2.54 * y_pixels_per_inch);

    GlobalUnlock(h_mem);
    GlobalFree(h_mem);

    // 
    DOCINFO doc_info;
    memset(&doc_info, 0, sizeof(DOCINFO));
    doc_info.cbSize = sizeof(DOCINFO);
    doc_info.lpszDocName = input_file_name;
    doc_info.lpszOutput = L"_mpki.pdf";  // output file name

    StartDoc(hdc_printer, &doc_info);
    StartPage(hdc_printer);

    // Print EMF
    // Put an image at the left-bottom of a page.
    RECT rect = {
        0, 
        pixel_size_page_y - pixel_size_emf_y, 
        pixel_size_emf_x, 
        pixel_size_page_y
    };
    PlayEnhMetaFile(hdc_printer, h_emf, &rect);

    // Close handles
    DeleteEnhMetaFile(h_emf);
    EndPage(hdc_printer);
    EndDoc(hdc_printer);
    DeleteDC(hdc_printer);
    ClosePrinter(h_printer);


    // Crop
    vector<BYTE> in_data;
    vector<BYTE> out_data;
    if (!LoadBinary(&in_data, "_mpki.pdf")) {
        printf("Failed reading a temporal file.");
        return 1;
    }
    size_t pos = 0;
    while(pos < in_data.size()){
        string str_line;
        while (pos < in_data.size()) {
            BYTE c = in_data[pos];
            str_line.push_back(c);
            pos++;
            if (c == '\n') {
                break;                
            }
        }
        // Modify MediaBox and CropBox
        // A unit of the positions is pt (1/72 inchi)
        if (str_line.find("/MediaBox") == 0) {
            str_line = Format("/MediaBox [ 0 0 %lf %lf ]\n", inchi_size_emf_x * 72, inchi_size_emf_y * 72);
        }
        else if (str_line.find("/CropBox") == 0) {
            str_line = Format("/CropBox [ 0 0 %lf %lf ]\n", inchi_size_emf_x * 72, inchi_size_emf_y * 72);
        }
        out_data.insert(out_data.end(), str_line.begin(), str_line.end());
    }
    if (!SaveBinary(out_data, "mpki.pdf")) {
        printf("Failed writing an output file.");
        return 1;
    }

    return 0;
}

