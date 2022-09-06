//---------------------------------------------------------------------------
#pragma hdrstop
#include <vcl.h>
#include "CalcTrafficIntens.h"
#include <sstream>
#include <fstream>
#include <cmath>
#include <iomanip>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "cspin"
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{

}
//---------------------------------------------------------------------------
using namespace std;
const int minWidth = 12;
enum mistakes  //��� ��������� ����������
{
	allGood, fileOpenError, lackOfMemoryErr
};
//---------------------------------------------------------------------------
//�������� �� private � public, ����� ����� ���� ������������ ������� ��������, ������������ �.�.
#ifdef SUBST_SG
class TMyStringGrid: TStringGrid
{
	public:
		 using TStringGrid::DeleteColumn;
		 using TStringGrid::MoveColumn;
		 using TStringGrid::InvalidateCell;
};
#endif
//---------------------------------------------------------------------------
// �������� ��� ������� �� ������ "�������"
void __fastcall TForm1::btYearCountClick(TObject *Sender)
{
	if(teYearCount->Text == L"")//���� � ���� �����, �� ������� �� ���� ������������ ����� ������ ���
	{
		StatusBar->Panels->Items[0]->Text = L"������� ������ ������!";
		return;
	}

	int toStrValue;//���������� ��� �������������� ����-� ������ � �������������

	if(!TryStrToInt(teYearCount->Text, toStrValue) || toStrValue <= 0 || toStrValue > 15)//TryStrToInt-����������� ������ � ������������� ����-�
	{
		if(toStrValue > 15)//���� ������� ������ 50-�� ���
		{
			StatusBar->Panels->Items[0]->Text = L"������������ �������� ����: 15";
			return;
		}
		StatusBar->Panels->Items[0]->Text = L"������! ������� ������������ ��������: *"   //����� ������� ,��� �������� �����������
		+ teYearCount->Text+L"*";
		return;
	}

	if(strgrData->ColWidths[1] == -1)//���� ��� ���������� ������ ������� ������� -1(����������, ��� �� �����)
	{
		strgrData->ColWidths[1] = strgrData->DefaultColWidth;//��������� ��� ��������� ������
		strgrData->Cols[1]->Clear();//���� �������
		strgrData->Cells[1][0] = 1;//������ 1 � �������, ��� 1-� ���
		strgrData->Options = strgrData->Options << goColSizing;
	}

	if((StrToInt(teYearCount->Text) < strgrData->ColCount - 1))//��������, ���� �� ������ ��� �������� �� ������
	{
		for(int i = StrToInt(teYearCount->Text); i < strgrData->ColCount - 1; i++)
			if(strgrData->Cells[strgrData->Col][strgrData->Row] != L"") //�������� �� ������� ������ � ��������� ������
			{
				StatusBar->Panels->Items[0]->Text = (L"� ������� � " + IntToStr(StrToInt(teYearCount->Text) + 1) + L" �� "
				+ IntToStr(strgrData->ColCount - 1) +
				 L" ��������� ������! ��� ����� ���� �������! ����������?");

				switch(Application->MessageBox((L"� ������� � " + IntToStr(StrToInt(teYearCount->Text) + 1) + L" �� "
				+ IntToStr(strgrData->ColCount - 1) +
				 L" ��������� ������! ��� ����� ���� �������! ����������?").w_str(), (Form1->Caption).w_str(),
				 MB_YESNO | MB_ICONQUESTION))
				{
					 case IDYES: break;
					 case IDNO: return;
				}
				break;
			}
	}

	if(StrToInt(teYearCount->Text) > strgrData->ColCount - 1)
		for(int i = strgrData->ColCount; i <= StrToInt(teYearCount->Text); i++)
		{
			strgrData->Cols[i]->Clear();
			strgrData->Cells[i][0] = i;
		}

	strgrData->ColCount = StrToInt(teYearCount->Text) + 1;

	btEditDelete->Enabled = true;
	menuSave->Enabled = true;
	menuSaveAs->Enabled = true;
    btRezSaveAs->Enabled = true;
	btRezCalculate->Enabled = true;
	menuRezCalculate->Enabled = true;
}
//---------------------------------------------------------------------------
mistakes DataSave(const String &Fname, const DynamicArray<DataStruct> &Data)
{
	fstream f(Fname.c_str(), ios::binary | ios::out);
	if(!f.is_open())//���� �� ������� ������� ����, ��������� ������
		return fileOpenError;

	int countPair = Data.Length;//���������� ��� ���-�� ���
	f.write((char *)&countPair, sizeof(countPair));
	f.write((char *)&Data[0], sizeof(Data[0]) * countPair);

	if(f.fail())//���� �� �������
	{
		f.close();//�������� �����, ��������� ������ �������� ������
		return lackOfMemoryErr;
	}
	f.close();//���� �� ������  ��������� � ���������� ���������, ��� �� ������
	return allGood;
}
//---------------------------------------------------------------------------
/*������� ��� ������������ ������������ ������������� ������������ ��� ������ �
 ������. ������� ���� ��������� �� StringGrid + ���������� ��� ������� �������*/
int widthCalcForStrgrData(TStringGrid *strgr,int cIndex)
{
	 if(!strgr)	//���� �� ���������� ���������������� �������, ������ �����
			return 0;
		strgr->Canvas->Font->Assign(strgr->Font);
		int NeedWidth = 0;
		for(int i = 0; i < strgr->RowCount; i++)
		{
			int textWidth = strgr->Canvas->TextWidth(strgr->Cells[cIndex][i]);
			if(textWidth > NeedWidth)
				NeedWidth = textWidth;
		}
		return NeedWidth + minWidth;
}
 /*������� ������������ ������ ������� ������� � ������� ���������� ������
   �� WidthCalcForStrgrData */
int transformCells(TStringGrid *strgr)
{
	if(!strgr)
		return 0;
	int totalWidth = 0;
	for(int i = 1; i < strgr->ColCount; i++)
	{
		int colWidth = widthCalcForStrgrData(strgr, i);
		strgr->ColWidths[i] = colWidth;
		totalWidth += colWidth;
	}
	return totalWidth;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
    strgrData->Options = strgrData->Options >> goColSizing;
	strgrData->ColWidths[0] = 310;//����� ������ ������ ����� ���� ���������
	memLinearApr->Lines->Text=L"";
	strgrData->Cells[0][0] = L"����";
	strgrData->Cells[0][1] = L"N ���/���";
	strgrData->Cells[0][2] = L"N ���/��� - �������������";
	strgrData->Cells[0][3] = L"���������� ������� �������������";
	strgrData->Cells[0][4] = L"������������� ������� �������������";
	//������, �� ���������� ��� ������ � ����������� ������������� ����(??)
	strgrData->Cells[0][5] = L"������ �������� �������������";
	strgrData->Cells[0][6] = L"����������� �������� ����������";
	strgrData->Cells[0][7] = L"������������������ ������� N";
	strgrData->ColWidths[1] = -1;//�������, ����� �� ���� �����
}
//---------------------------------------------------------------------------
void __fastcall TForm1::btEditFontClick(TObject *Sender)
{
	///
}
//---------------------------------------------------------------------------
void __fastcall TForm1::btEditAddClick(TObject *Sender)
{
	if(strgrData->ColWidths[1] == -1 || strgrData->ColCount > 15)
	{
		teYearCount->Text = strgrData->ColCount-1;
		strgrData->ColWidths[1] = strgrData->DefaultColWidth;
		if(cbCorrectColumnWidth->Checked)//��������, ����������� �� ������� �� ���������� �������� �����
			strgrData->Options = strgrData->Options << goEditing;
		//cbCorrectColumnWidth(NULL); ????

		strgrData->Cols[1]->Clear();//����� ����� ��������� ������ � ��������� � ���������� �����, ������� ������ �� ����
		strgrData->Cells[1][0] = 1; //����� ��������� 1 ��� ������ �������
	}
	else
	{   if (strgrData->ColCount < 16)//����� �� ���� ������ 15-�� ���, � ��������� ������ ������ �� ������
		{
        strgrData->Options = strgrData->Options << goColSizing;
		teYearCount->Text = strgrData->ColCount;
		strgrData->ColCount++;//�������� ������
		strgrData->Col = strgrData->ColCount - 1;//���������
		strgrData->Cols[strgrData->ColCount - 1]->Clear();//������
		strgrData->Cells[strgrData->ColCount - 1][0] = strgrData->ColCount - 1;
		strgrData->Cells[strgrData->ColCount - 1][0] = IntToStr(strgrData->ColCount - 1);
		strgrData->SetFocus();
		strgrData->Row = 1;
		}
	}

	btEditDelete->Enabled = true;
	menuSaveAs->Enabled = true;
    btRezSaveAs->Enabled = true;
	btRezCalculate->Enabled = true;
	menuRezCalculate->Enabled = true;
	menuSave->Enabled = true;
    menuRezCalculate->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::btEditDeleteClick(TObject *Sender)
{
	if(strgrData->ColWidths[strgrData->Col] == -1)//���� ������ ���

		return;

	if(strgrData->ColCount == 2)//(2-����������� ��������)
	{
		if(strgrData->Cells[strgrData->Col][1] != L"")//���� � ������ ���� �����-���� ������
		{
		StatusBar->Panels->Items[0]->Text =(L"� ������� " + IntToStr(strgrData->Col)
		 + " ��� ���� ������. �������?");

		switch (Application->MessageBox((L"� ������� " + IntToStr(strgrData->Col)
		 + " ��� ���� ������. �������?").w_str(),
		Form1->Caption.w_str(), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
		{
			case IDYES:
				break;
			case IDNO:
				return;
		}
		}
		strgrData->ColWidths[1] = -1;//����� �� ���� ����� ���� �������(�.� ������� ��� 2-� ���������)
		teYearCount->Text = 0;
		btEditDelete->Enabled = false;
		btRezCalculate->Enabled = false;
		btRezSaveAs->Enabled = false;
		strgrData->Options = strgrData->Options >> goEditing;//�������//�������� �����
		strgrData->Options = strgrData->Options >> goColSizing;
		return;
	}

	if(strgrData->Cells[strgrData->Col][1] != L""){//���� ��������� > 2

		StatusBar->Panels->Items[0]->Text =(L"� ������� " + IntToStr(strgrData->Col)
		 + " ��� ���� ������. �������?");

		switch (Application->MessageBox((L"� ������� " + IntToStr(strgrData->Col)
		 + " ��� ���� ������. �������?").w_str(),
		Form1->Caption.w_str(), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
		{
			case IDYES:
				break;
			case IDNO:
				return;
		}
	}


	strgrData->Cols[strgrData->Col]->Clear();//������ ������
	((TMyStringGrid*)strgrData)->DeleteColumn(strgrData->Col);//���������� � ������� ��������
	teYearCount->Text = strgrData->ColCount-1;////

	btRezCalculate->Enabled = true;
	menuSave->Enabled = true;
	menuSaveAs->Enabled = true;
    btRezSaveAs->Enabled = true;
	menuRezCalculate->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::menuOpenFClick(TObject *Sender)
{
	if(menuSave->Enabled == true)//��������, ���� �� � ������� ���-�� �� ������ ������
	{
		int msg; //���������� ��� ������ ��������� �� ������
		if(StatusBar->Panels->Items[1]->Text == L""){
			StatusBar->Panels->Items[0]->Text =(L"�������� ������������� ������!"
			" ��������� �� � ����� ����?");

			msg = Application->MessageBox(L"�������� ������������� ������!"
			" ��������� �� � ����� ����?",
			L"���������� ������������� ��������", MB_YESNOCANCEL | MB_ICONQUESTION//�������� ������ ����
			| MB_DEFBUTTON3);
		}
		else
			msg = Application->MessageBox((L"������, ������� ��������� �� �����, \""
			 + StatusBar->Panels->Items[1]->Text + "\" ���� ��������!" +
			 " C�������� �� � ������ �� ����?").w_str(),
			L"���������� ������������� ��������", MB_YESNOCANCEL | MB_ICONQUESTION
			| MB_DEFBUTTON3);

		switch(msg)
		{
			case ID_YES://���� ������������ ������� ��������� ������, ������� ������ ����������
			{
				menuSave->Click();
			};
			case ID_NO://�� ���, ��� ���
			{
				break;
			}
			case ID_CANCEL://���� ����� ��������
			{
				return;
			}
		}
	}

	if(!OpenFile->Execute())//���� �� �������� ����
		return;

	fstream f((OpenFile->FileName).c_str(), ios::in);

	if(f.peek() == EOF)//�������� �� ������ ����
	{
		StatusBar->Panels->Items[1]->Text = L"��������� ���� ����.";
		Application->MessageBox(L"��������� ���� ����.", Form1->Caption.w_str(),
		 MB_OK | MB_ICONEXCLAMATION);
		f.close();
		return;
	}
	StatusBar->Panels->Items[1]->Text = OpenFile->FileName;//������� ��� ������������ �����

	int countPair = 0 ;//���������� ��� ���-�� ��� ��� � �������������
	try
	{
		f.read((char *)&countPair, sizeof(countPair));
		Data.Length = countPair;
		f.read((char*)&Data[0], sizeof(Data[0]) * countPair);

		if(f.peek() != EOF)//�������� �� ������������ ����
		{
			StatusBar->Panels->Items[1]->Text = L"������! ������ ����� ���� ����������.";
			Application->MessageBox(L"������! ������ ����� ���� ����������.",
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			f.close();//��������� ����
			return;
		}
	}
	catch(...)
	{
		StatusBar->Panels->Items[1]->Text = L"������ ������ �����.";
		Application->MessageBox(L"������ ������ �����.", Form1->Caption.w_str(),
		MB_OK | MB_ICONERROR);
		f.close();
		return;
	}
	f.close();//���������
	strgrData->ColCount = countPair + 1; //������ ��� ���� ������� �������������

	if(strgrData->ColWidths[1] == -1)//���� ������ ������� �����
	{
		strgrData->ColWidths[1] = strgrData->DefaultColWidth;//��������� ����������� ��������
		strgrData->Options = strgrData->Options << goColSizing;
		if(cbEditingM->Checked)//���� ����� ������� �� �������������� �����
			strgrData->Options = strgrData->Options << goEditing;
	}

	for(int i = 1; i < countPair + 1; i++)
	{
		strgrData->Cols[i]->Clear();//������� ������ ������
		strgrData->Cells[i][0] = IntToStr(Data[i - 1].Year);//������ ���
		//(Data-1)- ������ ��� �������� ���� � 0, � � ����� �������� � 1
		if(Data[i - 1].Intensity == -1)//���� ������ ���, �� ����������
			strgrData->Cells[i][1] = L" ";
		else
		strgrData->Cells[i][1] = IntToStr(Data[i - 1].Intensity);//����� ���������
	}
		if(cbEditingM->Checked)
			cbEditingM->Checked = false;

    if(cbCorrectColumnWidth->Checked || menuCorrectColumnWidth->Checked )//���� ����� �������
	{
		if(strgrData->ColWidths[1] == -1)//���� ��� ������
			return;

		transformCells(strgrData);//������� � transformCells ������ strgrData
	}
	teYearCount->Text = strgrData->ColCount-1;////
	btEditDelete->Enabled = true;
	menuNew->Enabled = true;//��� ����� ������� �� �� ��������, � ���� ����������� ������� �����
	menuSave->Enabled = false;//������ ��� ������ ��� �������, ����� ���������
	menuSaveAs->Enabled = true;
	btRezSaveAs->Enabled = false;
	btRezCalculate->Enabled = true;
}

//---------------------------------------------------------------------------
 void __fastcall TForm1::strgrDataSetEditText(TObject *Sender, int ACol, int ARow,
          const UnicodeString Value)
{
	if(!menuSave->Enabled && CCText != Value && !strgrData->EditorMode)//���� �� ������� ������ ���������� � ��������� ����� � ������
	{
		menuSave->Enabled = true;
		menuSaveAs->Enabled = true;
        btRezSaveAs->Enabled = true;
		btRezCalculate->Enabled = true;
	}
	if(cbCorrectColumnWidth->Checked)//���� ����� ������� �� ����-�� ������
		strgrData->ColWidths[ACol] = widthCalcForStrgrData(strgrData, ACol);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::strgrDataSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)

{
	if(ARow > 1)
		strgrData->Options = strgrData->Options>>goEditing;//�������

	if(ARow == 1 && cbEditingM->Checked == true)//��������
		strgrData->Options = strgrData->Options<<goEditing;

	int val;//���������� ��� �������� �� ������ � ���������� ���� int
	if(!TryStrToInt(strgrData->Cells[strgrData->Col][strgrData->Row], val) || val <= 0 || val > 100000)
	{//�������� ��������� ������ � ��������
		if(val > 100000)//���� ������ ������������� �����, �� ������ � ���������
		{
			StatusBar->Panels->Items[0]->Text = L"������! �������� ������������� �� ������"
			" ��������� 100000";
			return;
		}
	}
//���� ������� ������ � ������� ����� �������������� � ������ � ������ ��������
//��� ���� ������� ����� �������������� � ������ � ������ ��������
	if((!menuSave->Enabled && strgrData->EditorMode &&
	strgrData->Cells[strgrData->Col][strgrData->Row] != CCText) ||
	(strgrData->EditorMode &&
	strgrData->Cells[strgrData->Col][strgrData->Row] != CCText))
	{
		menuSave->Enabled = true;
		menuSaveAs->Enabled = true;
        btRezSaveAs->Enabled = true;
		btRezCalculate->Enabled = true;
		menuRezCalculate->Enabled = true;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btRezCalculateClick(TObject *Sender)
{
	for(int i = 1; i < strgrData->ColCount; i++)
	{
		int toStrValue;//���������� ��� �������������� ����-� ������ � �������������
		//���� � ��������� ������ ���� ������
		if( !TryStrToInt(strgrData->Cells[i][1], toStrValue) || toStrValue <= 0 || toStrValue > 100000 ||
		strgrData->Cells[i][1] == L"" || strgrData->ColCount <= 2)
		{
			strgrData->Col = i;//���� �� ��������� � ������
			strgrData->Row = 1;//���� ������������ ������� ������ ������, � ������� ��� ������, ��� ��������� � �� , ��� ������
			StatusBar->Panels->Items[0]->Text = L"������� ������������ ������ ��� ������ �����������.";
			Application->MessageBox(L"������� ������������ ������ ��� ������ �����������.",
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			return;
		}
	}
	aprGr->Series[0]->Clear();//������ ������ � ������ ������� ��� ����������� ���-�
	aprGr->Series[1]->Clear();

	//���� ����� ������� �� ����� ���-�� ��������� -1
	if(Data.Length != strgrData->ColCount - 1 || menuSave->Enabled == true )
	{
		Data.Length = strgrData->ColCount - 1;//�� ������ ��� �������
		for(int i = 0; i < Data.Length; i++)//�� ������ � �� ����� ������� �� ������� ����������� ��������(��� � �������������)
		{
			Data[i].Year = StrToInt(strgrData->Cells[i + 1][0]);
			Data[i].Intensity = StrToInt(strgrData->Cells[i + 1][1]);
		}
	}
/*==============================================================================*/
	//���������� ������������������� ������ ��������� N(������� ����������� ��� ���������� a � b)
	int sumIntensity = 0, sumSquares = 0, sumYear = 0, sumPrIntensityYear = 0,
		prValSumSquares = 0, prIntensityYearOnPrYearIntensitu = 0 , i = 0, p = Data.Length;
	for(i = 0; i < p; i++)
	{
		sumYear = sumYear + (i+1);//����� �����(+1, �.� �������� ���� � 1)
		sumIntensity += Data[i].Intensity;//����� ��������������
		sumSquares += pow(i + 1, 2);//����� ���������(i+1 �.� �������� ���� � 1^2)
		sumPrIntensityYear += Data[i].Intensity * (i + 1);//����� ������������ ������������� � ����
	}
	prValSumSquares = p * sumSquares;//������������ ���-�� �������������� �� ����� ���������
	prIntensityYearOnPrYearIntensitu = p * sumPrIntensityYear;//������������ ���-�� ��� (�������������+���) �� ����� ������������ ������������� � ����
/*==============================================================================*/
	float a = 0, b = 0; //��� ������ � ���� (�� ������� a � b)
	a = (sumIntensity * sumSquares - sumYear * sumPrIntensityYear) / float(prValSumSquares - pow(sumYear, 2));
	b = (prIntensityYearOnPrYearIntensitu - sumYear * sumIntensity) / float(prValSumSquares - pow(sumYear, 2));
	//��������� ���������� �����(� float), �.�. �������� �� �����

	//����� � ���. ���� ������� �������� ������������� � ������������� � ��� ���������� ���������� � � b
	if(b > 0)// b>0 - ������� �������
	{
		memLinearApr->Lines->Text = L"�������:\r\n N = a + b * t = " + FloatToStrF(a,ffNumber,10,3)
		+ L" + " + FloatToStrF(b,ffNumber,10,3) + L"*t";
	}
	/*FloatToStrF - ������������ �������� � ��������� ������� � ������������� ������(�� Delphi)
	  ffNumber(���� � FloatToStrF) - �������� ������, ��� ������ ����� ������ ����� ��������� ����������� �����  */
	if(b == 0)// b=0 - ������� ��� b
	{
		memLinearApr->Lines->Text = L"������� N:\r\n N = a = " + FloatToStrF(a,ffNumber,10,3);
	}

	if(b < 0)// b<0 - ���� ����� �� ������(�.� �����)
	{
		memLinearApr->Lines->Text = L"�������:\r\n N = a + b * t = " + FloatToStrF(a,ffNumber,10,3)
	   + L' - '	+ FloatToStrF(b,ffNumber,10,3) + L"*t";
	}

	DynamicArray <int> Appr;//������ ������������ ������ ��� �������� ����������������� ��������(�������������)
	Appr.Length = p; //���������� Appr ����� Data.Length
	for(i = 0; i < p; i++)
	{
		Appr[i] = a + b * (i + 1);//�������� ������������ ������ NAppr �� �������
		strgrData->Cells[i + 1][2] = IntToStr(Appr[i]);//������ ���������� �������� �� ������ ������, ������� � ������� ����
	}
/*==============================================================================*/
	//������ ����������� � �������������� �������� �������������
	int absIntensity[p];				//������ ������� ������ ��� �������� ����������� ��������
	double relativeIntensity[p];		//������ ������� ������ ��� �������� �������������� ��������
	for(i = 0; i < p; i++)
	{
		if(i == p - 1)
			break;
		//(m - 1)������ ��� ������ � ������� � ���������, � �� �� ���������� � 0, � Data.Length ������ ����� ��� ����� 0, ��� ������ �������
		absIntensity[i] = Data[i + 1].Intensity - Data[i].Intensity;//���������� ���������� ������� �������������
		relativeIntensity[i] = (absIntensity[i] / float(Data[i + 1].Intensity)) * 100;//���������� ����� int � float
		strgrData->Cells[i + 2][3] = IntToStr(absIntensity[i]);//����������� ������� ������������� � ������������� �������� � ���������� �����-� ������
		strgrData->Cells[i + 2][4] = FloatToStrF(relativeIntensity[i], ffNumber, 10, 1) + L"%";//����������� �������� � ��������� ������� � ������
	}
	strgrData->Cells[1][3] = L"0";//��� ������� �������� ��������� ���������� , ������� ������ �� ����
	strgrData->Cells[1][4] = L"0";
/*==============================================================================*/
   //������ ������� �������������
	float avgIntensity = 0, AvgAppr = 0;
	for(i = 0; i < p; i++)
	{
		avgIntensity += Data[i].Intensity;//������ ����� ��������������
		AvgAppr += Appr[i];//������ ����� �������������
	}
	avgIntensity = avgIntensity / p;//������ ������� �������� �������������
	AvgAppr = AvgAppr / p;  //������ ������� �������� ������������� �������������
/*==============================================================================*/
	//������ ������������������� �������� �������������(avgSpreadIntensity)
	float squaredDeviation[p], dispersion = 0, avgSpreadIntensity, avgSpreadAppr;
	for(i = 0; i < p; i++)
	{
		squaredDeviation[i] = pow(Data[i].Intensity - avgIntensity, 2);
		dispersion += squaredDeviation[i];
	}
	dispersion = dispersion / p;
	avgSpreadIntensity = sqrt(dispersion);
	// ������ ������������������� �������� ������������� ��� ������������������ ��������(avgSpreadAppr)
	for(i = 0; i < p; i++)
	{
		squaredDeviation[i] = pow(Appr[i] - AvgAppr, 2);
		dispersion += squaredDeviation[i];
	}
	dispersion = dispersion / p;
	avgSpreadAppr = sqrt(dispersion);
	memLinearApr->Lines->Add(L"������������������ ������� ������������� N: "
	 + FloatToStrF(avgSpreadAppr, ffNumber, 10, 3));//������ � ����-� memo

	 strgrData->Cells[1][7] = FloatToStrF(avgSpreadAppr, ffNumber, 10, 3);//������, �� ���� ��� ����, ����� �������� � ���� ��� ����������
/*==============================================================================*/
	//������ ��������� ������������ ���������� ����� ��������� � ����������� ����� ���������� �������
	float sumPrOtklIn = 0;
	double linCoefCorr;
	for(i = 0; i < p; i++)
		sumPrOtklIn += (Data[i].Intensity - avgIntensity)*(Appr[i] - AvgAppr);
	if(avgSpreadAppr == 0 || avgSpreadIntensity == 0)
		linCoefCorr = 0;  //������ ��� ��������� ������� �� 0
	else
		linCoefCorr = sumPrOtklIn /(p * avgSpreadAppr * avgSpreadIntensity);

	memLinearApr->Lines->Add(L"����������� �������� ����������: "
	+ FloatToStrF(linCoefCorr, ffNumber, 10, 3));

	strgrData->Cells[1][6] = FloatToStrF(linCoefCorr, ffNumber, 10, 3);//������, �� ���� ��� ����, ����� �������� � ���� ��� ����������
/*==============================================================================*/
	//������ ������������������ ������ �������� ������������� � ��������� �� �������� �������� �������������
	float misLinAppr;
	Appr[0]=1;
	for(i = 0; i < p; i++){
		misLinAppr = abs(Appr[i] - avgIntensity)/(Appr[i] * p) * 100;
		strgrData->Cells[i+1][5] = FloatToStrF(misLinAppr, ffNumber, 10, 1) + L"%";
	}

	memLinearApr->Lines->Add(L"������ �������� �������������: " +
	FloatToStrF(misLinAppr, ffNumber, 10, 1) + L"%");

	strgrData->Cells[1][5] = L"0";//�.� � ������ ����

	// ������ ������������� �������� ������������� � ������������ �������� ��������������
	for(i = 0; i < p; i++)
	{
		aprGr->Series[0]->Add(Data[i].Intensity, i + 1);
		aprGr->Series[1]->Add(Appr[i], i + 1);
	}
    if(cbCorrectColumnWidth->Checked || menuCorrectColumnWidth->Checked )//���� ����� �������
	{
		if(strgrData->ColWidths[1] == -1)//���� ��� ������
			return;

		transformCells(strgrData);//������� � transformCells ������ strgrData
	}
	btRezCalculate->Enabled = false;
	menuRezCalculate->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::cbCorrectColumnWidthClick(TObject *Sender)
{
	if(Sender==cbCorrectColumnWidth)
		menuCorrectColumnWidth->Checked=cbCorrectColumnWidth->Checked;
	else{
        cbCorrectColumnWidth->OnClick=NULL;
		try{
			cbCorrectColumnWidth->Checked=menuCorrectColumnWidth->Checked;
		}
		__finally{
			cbCorrectColumnWidth->OnClick=cbCorrectColumnWidthClick;
		}
	}
	if(cbCorrectColumnWidth->Checked || menuCorrectColumnWidth->Checked )//���� ����� �������
	{
		if(strgrData->ColWidths[1] == -1)//���� ��� ������
			return;

		transformCells(strgrData);//������� � transformCells ������ strgrData
	}

}
//---------------------------------------------------------------------------

void __fastcall TForm1::cbEditingMClick(TObject *Sender)
{
	if(cbEditingM->Checked == false)//���� ������� ���, �� �������� � ��������
		strgrData->Options = strgrData->Options >> goEditing;
	else
		strgrData->Options = strgrData->Options << goEditing;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::menuExitClick(TObject *Sender)
{
	Form1->Close();//��� ������� �� ������, ������ ��������� �����
}
//---------------------------------------------------------------------------

void __fastcall TForm1::menuNewClick(TObject *Sender)
{
	if(menuSave->Enabled == true)//���� ������ ���������� �������(���� �����-�� ������)
	{
		int msg;//����� ���������� ��� ������ ����
		if(StatusBar->Panels->Items[1]->Text == L""){
			msg = Application->MessageBox(L"�������� ������������� ������!"
			" ��������� �� � ����� ����?",
			L"���������� ������������� ��������", MB_YESNOCANCEL | MB_ICONQUESTION//�������� ������ ����
			| MB_DEFBUTTON3);
		}
		else
			msg = Application->MessageBox((L"������, ������� ��������� �� �����, \""
			 + StatusBar->Panels->Items[1]->Text + "\" ���� ��������!" +
			 " C�������� �� � ������ �� ����?").w_str(),
			L"���������� ������������� ��������", MB_YESNOCANCEL | MB_ICONQUESTION
			| MB_DEFBUTTON3);
		switch(msg)
		{
			case ID_YES://���� ������������ ������� ���������, �� ���������(������� ������ ������)
			{
				menuSave->Click();//����� ������ ����������
			};
			case ID_NO://�� �����, ��� �� �����...
			{
				break;
			}
			case ID_CANCEL:
			{
				Abort();//��������� ����������
			}
		}
	}

	//������� ��������(��������� �� � ������� �����)
	for(int i = 1; i < strgrData->ColCount; i++)
	{
		strgrData->Cols[i]->Clear();//������ ������ �������
		strgrData->Cells[i][0] = IntToStr(i);//���������� � ���������� ������� ������ ���������
	}
	strgrData->ColCount = 2;//�������� ������ 2 ��������(���� ��� ������, ������ ��� ��������)
	strgrData->ColWidths[1] = -1;//������� ����
	strgrData->Options = strgrData->Options >> goEditing;//������� ����������� ��������������
	strgrData->Options = strgrData->Options >> goColSizing;//������� ����������� ��������� ������

	teYearCount->Clear();//������ ���� ��� ����� ���-�� ���
	StatusBar->Panels->Items[1]->Text = L"";//������ ���������
	StatusBar->Panels->Items[0]->Text = L"";
	memLinearApr->Lines->Text = L"";//��������� ������ �������������� ����
	Data.Length = 0;//������� �����, ��� ����������� �������, ��� �������� ������������ ������
	aprGr->Series[0]->Clear();
	aprGr->Series[1]->Clear();
	if(!cbEditingM->Checked)//���� ������ �������������� �� �������, ������������
		cbEditingM->Checked = true;

	btRezSaveAs->Enabled = false;
	menuRezCalculate->Enabled = false;
	btRezCalculate->Enabled = false;
	btEditDelete->Enabled = false;
	menuSave->Enabled = false;
	menuSaveAs->Enabled = false;
	menuDelete->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::menuEditFontClick(TObject *Sender)
{
  TFont *sgFont = new TFont(), *sgFontBefore = new TFont();
	int bRowHeight = strgrData->RowHeights[0];
	int bFColWidth = strgrData->ColWidths[0];
	try
	{
		sgFontBefore->Assign(strgrData->Font);
		if(FontDialog1->Execute())
		{
			strgrData->Font->Assign(FontDialog1->Font);
			strgrData->Canvas->Font->Assign(FontDialog1->Font);
			//��������� ������ �������������� �������
			strgrData->ColWidths[0] = strgrData->Canvas->TextWidth(strgrData->Cells[0][4]) + 8;

			//��������� ������ ��������
			if(strgrData->ColWidths[1] != -1)
				transformCells(strgrData);

			//��������� ������ �����
			for(int i = 0; i < strgrData->RowCount; i++)
				strgrData->RowHeights[i] = strgrData->Canvas->TextHeight(strgrData->Cells[0][0]) + 4;
		}
		else
		{
			strgrData->Font->Assign(sgFontBefore);
			for(int i = 0; i < strgrData->RowCount; i++)
				strgrData->RowHeights[i] = bRowHeight;
			strgrData->ColWidths[0] = bFColWidth;

			if(strgrData->ColWidths[1] != -1)
				transformCells(strgrData);
		}
	}__finally{
		delete sgFont;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::strgrDataColumnMoved(TObject *Sender, int FromIndex, int ToIndex)

{
   if(FromIndex < ToIndex)//���� ��� �������� ������(���) ����������� ������ ������ ��� ���, � ������� ���������
		for(int i = ToIndex; i >= 1; i--)//���������������� �������� ����(���� ����� ���� �� ����� ����� 1)
			strgrData->Cells[i][0] = i; //��� ������� �����������������
	else
		for(int i = ToIndex; i <= FromIndex; i++)//���� ������(���) ������, ��� ���, � ������� ���������
			strgrData->Cells[i][0] = i;//��� ������� �����������������

		if(strgrData->Cells[FromIndex][1] != L"" || strgrData->Cells[ToIndex][1] != L"")//������������ ������ ���������� ����� ��������
		{
			menuSave->Enabled = true;
		}
     btRezCalculate->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::strgrDataDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect,
		  TGridDrawState State)
{
	//�������� ���� ����� � ����������� ��� �������� �������
	if(ARow > 1 && ARow < 6 && ACol > 0)//�� 1 �� 6 (�������) � �� 0 �������
	{
		Rect.Left -= 4;
		strgrData->Canvas->Brush->Color = clWebSilver;//�������� ����� ����
		strgrData->Canvas->FillRect(Rect);
		strgrData->Canvas->TextOut(Rect.left + 6, Rect.Top + 2,
		strgrData->Cells[ACol][ARow]);
	}
	//�������� ������������ � �� ������ �������� ������
	if(ARow == 1 && ACol > 0 && strgrData->Cells[ACol][ARow] != L"")//� ������ ������� � ������� �������� ������(+���� ��� ������ ����  ��������)
	{
		int toStrValue;
		if(!TryStrToInt(strgrData->Cells[ACol][ARow], toStrValue) || toStrValue <= 0 ||
		toStrValue > 100000)//���� ��������� ������ �����������, �� ����� � �������
		{
			Rect.Left -= 4;
			strgrData->Canvas->Brush->Color = clWebLightCoral;//�������� ������� ����
			strgrData->Canvas->FillRect(Rect);
			strgrData->Canvas->TextOut(Rect.left + 6, Rect.Top + 2,
			strgrData->Cells[ACol][ARow]);
        }
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::strgrDataKeyPress(TObject *Sender, System::WideChar &Key)

{
	if(!(Key >= L'0' && Key <= L'9' || Key == 9)
		&& Key != VK_BACK && Key != 3/*Ctrl+C*/ && Key != 22/*Ctrl+V*/
		&& Key != VK_RETURN && Key != VK_ESCAPE)
		{
			StatusBar->Panels->Items[0]->Text = L"������! ������ ������������ ������ '" +
			UnicodeString(Key) + L"'";
			Key = 0;
			return;
		}
		else StatusBar->Panels->Items[0]->Text = L"";//����� ��������

		if(Key == VK_ESCAPE && strgrData->EditorMode)//���� ������������ ������ esc, �� ������� ��������
		strgrData->Cells[strgrData->Col][strgrData->Row] = CCText;

			if(Key == VK_RETURN && strgrData->EditorMode)
	{
		int p;//���������� ��� ��������
		if(!TryStrToInt(strgrData->Cells[strgrData->Col][strgrData->Row], p) || p <= 0 || p > 100000)
		{
			if(p > 100000)
			{
				StatusBar->Panels->Items[0]->Text = L"������! ������������ �������� �������������: 100000";
				return;
			}
			if(strgrData->Cells[strgrData->Col][strgrData->Row] == L"")
				return;//���� ������ ��������� �������
			StatusBar->Panels->Items[0]->Text = L"������! ������������ ��������: " +
			strgrData->Cells[strgrData->Col][strgrData->Row];
			return;
		}

		if((strgrData->Col + 1) != strgrData->ColCount)
			strgrData->Col += 1;//��� ������� �� Enter ������� ������ ����� �� ��������� ������

		if((!menuSave->Enabled && strgrData->EditorMode
		&& strgrData->Cells[strgrData->Col][strgrData->Row] != CCText) ||
		 (strgrData->EditorMode && strgrData->Cells[strgrData->Col][strgrData->Row] != CCText))
		{
			menuSave->Enabled = true;
			menuSaveAs->Enabled = true;
			btRezSaveAs->Enabled = true;
			btRezCalculate->Enabled = true;
			menuRezCalculate->Enabled = true;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::teYearCountKeyPress(TObject *Sender, System::WideChar &Key)

{
	if(!(Key >= L'0' && Key <= L'9' || Key == 9)
			&&Key != VK_BACK && Key != 3 && Key != 22 && Key != 24 && Key != 26
			&& Key != VK_RETURN && Key != VK_ESCAPE && strgrData->Options.Contains(goEditing))
			{
				StatusBar->Panels->Items[0]->Text = L"������! ������ ������������ ������ ''" +
				UnicodeString(Key) + L"''";
				Key = 0;
				return;
			}
			else StatusBar->Panels->Items[0]->Text = L"";//����� ��������
}
//---------------------------------------------------------------------------

void __fastcall TForm1::strgrDataGetEditText(TObject *Sender, int ACol, int ARow,
          UnicodeString &Value)
{
	CCText = Value; //���� ������� ���������, �� �������� ������� ��������
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TimerTimer(TObject *Sender)
{
	StatusBar->Panels->Items[0]->Text = L"";
}

//---------------------------------------------------------------------------

void __fastcall TForm1::menuSaveValClick(TObject *Sender)
{   //�������� ��� ������� � ������ ������ � �� ���������
	//������ ��� ���������� �������
	SaveFile->Filter = L"��������� ���� (*.txt)|*.txt|��� ����� (*.*)|*.*";
	SaveFile->DefaultExt = L"txt";

	if(!SaveFile->Execute())//���� �� ���������� ���������
		return;
	ofstream f;//���������� ��� �����
	f.open((SaveFile->FileName).c_str(), ios::out);
	if(!f.is_open())//���� �� ���������� �������
	{
		Application->MessageBox(L"�� ������� �������� ������ � �����",
		 Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
		return;
	}

	for(int i = 0; i < strgrData->RowCount + 2; i++)//+2 �.�. ������ ��� ��� ����������(� ���� 2-� ������� ��� � �������)
	{
		f << setw(35) << left << AnsiString(strgrData->Cells[0][i]).c_str() << ':';//�������� + ����� ������
		for(int j = 1; j < strgrData->ColCount; j++)
		{
			if(strgrData->Cells[j][i] == L"")//���� ������ ��� � ������, �� ����� ��������
				continue;
			f << setw(10) << right << AnsiString(strgrData->Cells[j][i]).c_str();//������������ �� ������ �������� �������� � ������
		}
		f << endl;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::menuSaveGraphClick(TObject *Sender)
{
	SaveFile->Filter = "���� ����������� Bitmap (*.bmp)|*.bmp|���� ����������� MetaFile (*.emf)|*.emf|��� ����� (*.*)|*.*";

	if(!SaveFile->Execute())//���� �������� ������ ����������
		return;

	//��������� ������� �� ����������, ����������� � ���� ����������
	String format;//���������� ��� �������
	format = SaveFile->FileName;
	for(int i = 1; i <= format.Length(); i++)//���� ����� ����� ����� b, �� ������ bmp � �.�
	{
		if(format[i] == L'.' && format[i + 1] == L'b')
			SaveFile->FilterIndex = 1;
		if(format[i] == L'.' && format[i + 1] == L'e')
			SaveFile->FilterIndex = 2;
	}

	if(SaveFile->FilterIndex == 1)
	{
		SaveFile->DefaultExt = L"bmp";
		aprGr->SaveToBitmapFile(SaveFile->FileName);
	}
	if(SaveFile->FilterIndex == 2)
	{
		SaveFile->DefaultExt = L"emf";
		aprGr->SaveToMetafile(SaveFile->FileName);
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btEditInsertClick(TObject *Sender)
{

	if(strgrData->ColWidths[1] == -1)// ��������� ������ 1-�� ��������
	{
		teYearCount->Text = strgrData->ColCount-1;
		strgrData->ColWidths[1] = strgrData->DefaultColWidth;//��������� �������� ���������
		if(cbEditingM->Checked)//���� ����� ������� �� ������ ��������������
			strgrData->Options = strgrData->Options<<goEditing;
		strgrData->Options = strgrData->Options << goColSizing;
	}
	else
	{
		strgrData->ColCount++;//�������� �������
		((TMyStringGrid*)strgrData)->MoveColumn(strgrData->ColCount - 1, strgrData->Col);//�������� ������
		strgrData->Row = 1;
	}

	strgrData->Cells[strgrData->Col][0] = strgrData->Col;
	teYearCount->Text = strgrData->ColCount-1;
	strgrData->Col -= 1;
	btEditDelete->Enabled = true;
	menuDelete->Enabled = true;
	btRezCalculate->Enabled = true;
	menuRezCalculate->Enabled = true;
	menuSave->Enabled = true;
	menuSaveAs->Enabled = true;
	btRezSaveAs->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	if(menuSave->Enabled == true)
	{
		int mVal;
		if(StatusBar->Panels->Items[0]->Text == L"")
			mVal = Application->MessageBox(L"������� ������������� ����� ������. "
			"��������� � ����� ����?",
			Form1->Caption.w_str(), MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON3);
		else
			mVal = Application->MessageBox((L"������, ����������� �� �����\"" +
			 StatusBar->Panels->Items[0]->Text +
			L"\" ���� ��������. ������ �� �� ��������� �� � ��� ��� ����?").w_str(),
			Form1->Caption.w_str(), MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON3);
		switch(mVal)
		{
			case ID_YES:
			{
				menuSave->Click();
			};
			case ID_NO:
			{
				break;
			}
			case ID_CANCEL:
			{
				CanClose = false;
				break;
			}
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::btRezSaveAsClick(TObject *Sender)
{


	SaveFile->Filter = L"��������� ���� (*.txt)|*.txt|��� ����� (*.*)|*.*";
	SaveFile->DefaultExt = L"txt";

	if(!SaveFile->Execute())
		return;
	ofstream f;
	f.open((SaveFile->FileName).c_str(), ios::out);
	if(!f.is_open())
	{
		Application->MessageBox(L"�� ������� �������� ������ � �����",
		 Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
		return;
	}
    int i =0;
	for(i; i < strgrData->RowCount + 4; i++)
	{
		f << setw(36) << left << AnsiString(strgrData->Cells[0][i]).c_str() << ':';
		for(int j = 1; j < strgrData->ColCount; j++)
		{
			if(strgrData->Cells[j][i] == L"")
				continue;
			f << setw(10) << right << AnsiString(strgrData->Cells[j][i]).c_str();
		}
		f << endl;
	}
	btRezSaveAs->Enabled = false;
	menuSaveAs->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::menuSaveClick(TObject *Sender)
{
	for(int i = 1; i < strgrData->ColCount; i++)//�� 1 �� ���-�� ���������
	{
		if(strgrData->Cells[i][1] == L"")//������ ������ ��������
			continue;
		int toStrValue;//���������� ��� �������������� ����-� ������ � �������������
		if(!TryStrToInt(strgrData->Cells[i][1], toStrValue) || toStrValue <= 0)//���� ����� �� �������������, ������
		{
			strgrData->Col = i;
			strgrData->Row = 1; //����� �������� ������ � ���������, ������� �� 1
			StatusBar->Panels->Items[0]->Text = (L"������! ���������� ������������ ������ � ������� "
			+ IntToStr(i));
			Application->MessageBox((L"������! ���������� ������������ ������ � ������� "
			+ IntToStr(i)).w_str(),
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);//����� ��������������
			Abort();
		}
	}
	//����� ������
	SaveFile->Filter = L"���� � ������� ������������� (*.msw)|*.msw|��� ����� (*.*)|*.*";

	if(StatusBar->Panels->Items[1]->Text == "")//���� � ������ ���� ������ ���
	{
		if(!SaveFile->Execute())
			Abort();//���� �� ������� ���������, �� ������� ���������
			//Abort()-�������� ��������� ������ ���������, ��������� ������, ������� �������� ���������� ���������
		StatusBar->Panels->Items[1]->Text = SaveFile->FileName;//����� ����� ����� � ��� ������������ � ���������
	}

	if(menuSave->Enabled)//���� ������ ���������� ��������
	{
		Data.Length = strgrData->ColCount - 1;//������ ��� ���� ����� � �� ��������� � ����-��� ������
		for(int i = 0; i < Data.Length; i++)//�� 0 �� ������ Data
		{
			if(strgrData->Cells[i + 1][1] == L"" && i != Data.Length)//���� ������ ������� ����
				Data[i].Intensity = -1;
			else
			Data[i].Intensity = StrToInt(strgrData->Cells[i + 1][1]);//���� �� ������, �� �������� �������������
			Data[i].Year = StrToInt(strgrData->Cells[i + 1][0]);//���� �� ������, �� ��������
		}
	}

	switch(DataSave(StatusBar->Panels->Items[1]->Text, Data))
	{
		case allGood: break;
		case fileOpenError:
		{
			Application->MessageBox(L"������ �������� ����� ��� ������ ������.",
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			return;
		}
		case lackOfMemoryErr:
		{
			Application->MessageBox(L"������������ ����� ��� ���������� �����!",
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			return;
		}
	}

	menuSave->Enabled = false;//�������� ������, ��� ������ ��� ��������
	menuSaveAs->Enabled = true;
	btRezSaveAs->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::menuSaveAsClick(TObject *Sender)
{  //����� ���� �����, ��� � ���������
	for(int i = 1; i < strgrData->ColCount; i++)
	{
		if(strgrData->Cells[i][1] == L"") //�.�. ������ ������ ���� ����� ���������
			continue;
		int nval;
		if(!TryStrToInt(strgrData->Cells[i][1], nval) || nval<=0)
		{
			strgrData->Col = i;
			strgrData->Row = 1; // ���� �������� ������ ������ � ���������, �� ������ 1
			Application->MessageBox((L"������! ���������� ������������ �������� � ������� �"
			 + IntToStr(i)).w_str(),
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			Abort();
		}
	}

	SaveFile->Filter = L"���� � ������� ������������� (*.msw)|*.msw|��� ����� (*.*)|*.*";
	StatusBar->Panels->Items[1]->Text = L"";

	if(StatusBar->Panels->Items[1]->Text == "")//���� � ������ ���� ������ ���
	{
		if(!SaveFile->Execute())
			Abort();//���� �� ������� ���������, �� ������� ���������
			//Abort()-�������� ��������� ������ ���������, ��������� ������, ������� �������� ���������� ���������
		StatusBar->Panels->Items[1]->Text = SaveFile->FileName;//����� ����� ����� � ��� ������������ � ���������
	}

	if(menuSave->Enabled == true)
	{

		Data.Length = strgrData->ColCount - 1;
		for(int i = 0; i < Data.Length; i++)
		{
			if(strgrData->Cells[i + 1][1] == L"" && i != Data.Length)
				Data[i].Intensity = -1;
			else
			Data[i].Intensity = StrToInt(strgrData->Cells[i + 1][1]);
			Data[i].Year = StrToInt(strgrData->Cells[i + 1][0]);
		}
	}



	switch(DataSave(StatusBar->Panels->Items[1]->Text, Data))
	{
		case allGood: break;
		case fileOpenError:
		{
			Application->MessageBox(L"������ �������� ����� ��� ������ ������.",
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			return;
		}
		case lackOfMemoryErr:
		{
			Application->MessageBox(L"������! ������������ ����� ��� ����������"
			" ������", Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			return;
		}
	}

	menuSave->Enabled = false;
}
//---------------------------------------------------------------------------
int cmpNmb(const String &s1, const String &s2)
{
	return StrToInt(s2) - StrToInt(s1);
}

float sortPercent(const String &s1, const String &s2)
{
	String ss1 = s1, ss2 = s2;
	ss1.Delete(ss1.Length() - 1, ss1.Length());
	ss2.Delete(ss2.Length() - 1, ss2.Length());
	return StrToFloat(ss2) - StrToFloat(ss1);
}

 void sgSort(TStringGrid *sg, int rowIdx, bool ascending)
{
	TStrings *row = sg->Rows[rowIdx];
	TStrings *tmpCol = new TStringList();
	try
	{
			for (int i = sg->FixedCols; i < sg->ColCount; i++)
				for (int j = i; j < sg->ColCount; j++)
				{
					if(rowIdx != 4)
						if (cmpNmb(row->Strings[i], row->Strings[j]) >
						0 && ascending || cmpNmb(row->Strings[i],
						row->Strings[j]) < 0 && !ascending)
						{
							tmpCol->Assign(sg->Cols[i]);
							sg->Cols[i]->Assign(sg->Cols[j]);
							sg->Cols[j]->Assign(tmpCol);
						}
					if(rowIdx == 4)
						if(sortPercent(row->Strings[i], row->Strings[j]) >
						0 && ascending || sortPercent(row->Strings[i],
						row->Strings[j]) < 0 && !ascending)
						{
							tmpCol->Assign(sg->Cols[i]);
							sg->Cols[i]->Assign(sg->Cols[j]);
							sg->Cols[j]->Assign(tmpCol);
						}
				}

    }
    __finally {
        delete tmpCol;
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::strgrDataDblClick(TObject *Sender)
{
    TPoint mousePos;
	GetCursorPos(&mousePos);    //��������� ������� � �������� �����������
	mousePos = (strgrData->ScreenToClient(mousePos));   //�������������� �������� ��������� � ����������
	if(GetCursor() == Screen->Cursors[crHSplit])
	{
		TGridCoord gc = strgrData->MouseCoord(mousePos.X-4, mousePos.Y);
		int colWidth = widthCalcForStrgrData(strgrData, gc.X);
		if(strgrData->ColWidths[gc.X] != colWidth)
			strgrData->ColWidths[gc.X] = colWidth;
		else strgrData->ColWidths[gc.X] = minWidth;
	}
	 else
	{
		if(strgrData->ColWidths[1] == -1)
			return;
		TGridCoord gc = strgrData->MouseCoord(mousePos.X, mousePos.Y);
		if (gc.X == 0)
		{
			int i = abs(rowNmbAndSortKind) - 1; // ������ ������

			((TMyStringGrid*)strgrData)->InvalidateCell(0, i);
			for (int i = 1; i < strgrData->ColCount; i++)
				if (strgrData->Cells[i][gc.Y].IsEmpty())
				{
					rowNmbAndSortKind = 0;
					strgrData->Invalidate();
					return;
				}
			if (rowNmbAndSortKind && i == gc.Y)
				rowNmbAndSortKind = -rowNmbAndSortKind;
			else
				rowNmbAndSortKind = gc.Y + 1;
			i = gc.Y;

			strgrData->Cols[strgrData->Col]->Objects[0] = (TObject*)true;//��������� �������� ������ �� ������� �� ������� ��������� �����

			sgSort(strgrData, i, rowNmbAndSortKind > 0);
			strgrData->Invalidate();
			transformCells(strgrData);

			for(i = strgrData->FixedCols; i < strgrData->ColCount; i++)
				if(strgrData->Cols[i]->Objects[0])
				{
					strgrData->Cols[strgrData->Col]->Objects[0] = (TObject*)false;
					strgrData->Col = i;
				}
		}
	}
        btRezCalculate->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::OpenFileShow(TObject *Sender)
{
    ///
}
//---------------------------------------------------------------------------

