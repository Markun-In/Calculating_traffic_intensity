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
enum mistakes  //Для получения результата
{
	allGood, fileOpenError, lackOfMemoryErr
};
//---------------------------------------------------------------------------
//Повышаем из private в public, чтобы можно было использовать функции удаления, перемещенияи т.д.
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
// Действия при нажатии на кнопку "указать"
void __fastcall TForm1::btYearCountClick(TObject *Sender)
{
	if(teYearCount->Text == L"")//Если в поле пусто, то сказать об этом пользователь через статус бар
	{
		StatusBar->Panels->Items[0]->Text = L"Введена пустая строка!";
		return;
	}

	int toStrValue;//Переменная для преобразования знач-я строки в целочисленное

	if(!TryStrToInt(teYearCount->Text, toStrValue) || toStrValue <= 0 || toStrValue > 15)//TryStrToInt-преобразует строку в целочисленное знач-е
	{
		if(toStrValue > 15)//Если введено больше 50-ти лет
		{
			StatusBar->Panels->Items[0]->Text = L"Максимальное значение года: 15";
			return;
		}
		StatusBar->Panels->Items[0]->Text = L"Ошибка! Введено некорректное значение: *"   //Иначе вывести ,что значение некорректно
		+ teYearCount->Text+L"*";
		return;
	}

	if(strgrData->ColWidths[1] == -1)//Если при добавлении ширина первого столбца -1(стандартно, ибо он скрыт)
	{
		strgrData->ColWidths[1] = strgrData->DefaultColWidth;//Возвращаю ему дефолтную ширину
		strgrData->Cols[1]->Clear();//Чищу столбец
		strgrData->Cells[1][0] = 1;//Ставлю 1 в столбик, где 1-й год
		strgrData->Options = strgrData->Options << goColSizing;
	}

	if((StrToInt(teYearCount->Text) < strgrData->ColCount - 1))//Проверка, есть ли данные при нажатиии на кнопку
	{
		for(int i = StrToInt(teYearCount->Text); i < strgrData->ColCount - 1; i++)
			if(strgrData->Cells[strgrData->Col][strgrData->Row] != L"") //Проверка на наличие данных в удаляемых сроках
			{
				StatusBar->Panels->Items[0]->Text = (L"В ячейках с " + IntToStr(StrToInt(teYearCount->Text) + 1) + L" до "
				+ IntToStr(strgrData->ColCount - 1) +
				 L" находятся данные! Они могут быть утеряны! Продолжить?");

				switch(Application->MessageBox((L"В ячейках с " + IntToStr(StrToInt(teYearCount->Text) + 1) + L" до "
				+ IntToStr(strgrData->ColCount - 1) +
				 L" находятся данные! Они могут быть утеряны! Продолжить?").w_str(), (Form1->Caption).w_str(),
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
	if(!f.is_open())//Если не удается открыть файл, возвращаю ошибку
		return fileOpenError;

	int countPair = Data.Length;//Переменная для кол-ва пар
	f.write((char *)&countPair, sizeof(countPair));
	f.write((char *)&Data[0], sizeof(Data[0]) * countPair);

	if(f.fail())//Если не удалось
	{
		f.close();//Закрываю поток, возвращаю ошибку нехватки памяти
		return lackOfMemoryErr;
	}
	f.close();//Есди всё хорошо  закрываем и возвращаем результат, что всё хорошо
	return allGood;
}
//---------------------------------------------------------------------------
/*Функция для высчитывания необходимого максимального пространства для текста в
 ячейке. Передаю туды указатель на StringGrid + переменная для индекса столбца*/
int widthCalcForStrgrData(TStringGrid *strgr,int cIndex)
{
	 if(!strgr)	//Если не получилось инициализировать таблицу, просто выход
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
 /*Функция высчитывания ширины каждого столбца с помощью полученных данных
   из WidthCalcForStrgrData */
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
	strgrData->ColWidths[0] = 310;//Чтобы первые ячейки можно было прочитать
	memLinearApr->Lines->Text=L"";
	strgrData->Cells[0][0] = L"Годы";
	strgrData->Cells[0][1] = L"N авт/сут";
	strgrData->Cells[0][2] = L"N авт/сут - аппроксимация";
	strgrData->Cells[0][3] = L"Абсолютный прирост интенсивности";
	strgrData->Cells[0][4] = L"Относительный прирост интенсивности";
	//Скрыты, но существуют для вывода в сохраненный пользователем файл(??)
	strgrData->Cells[0][5] = L"Ошибка линейной аппроксимации";
	strgrData->Cells[0][6] = L"Коэффициент линейной корреляции";
	strgrData->Cells[0][7] = L"Среднеквадратичный разброс N";
	strgrData->ColWidths[1] = -1;//Скрываю, чтобы не было черты
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
		if(cbCorrectColumnWidth->Checked)//Проверка, установлена ли галочка на испралении размеров ячеек
			strgrData->Options = strgrData->Options << goEditing;
		//cbCorrectColumnWidth(NULL); ????

		strgrData->Cols[1]->Clear();//Чтобы после удалеения строки с значением и добавления новой, старого текста не было
		strgrData->Cells[1][0] = 1; //Чтобы появилась 1 над первой строкой
	}
	else
	{   if (strgrData->ColCount < 16)//Чтобы не было больше 15-ти лет, в противном случае ничего не делать
		{
        strgrData->Options = strgrData->Options << goColSizing;
		teYearCount->Text = strgrData->ColCount;
		strgrData->ColCount++;//Добавляю строку
		strgrData->Col = strgrData->ColCount - 1;//Перемещаю
		strgrData->Cols[strgrData->ColCount - 1]->Clear();//Очищаю
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
	if(strgrData->ColWidths[strgrData->Col] == -1)//Если ничего нет

		return;

	if(strgrData->ColCount == 2)//(2-стандартное значение)
	{
		if(strgrData->Cells[strgrData->Col][1] != L"")//Если в ячейке есть какой-либо символ
		{
		StatusBar->Panels->Items[0]->Text =(L"В столбце " + IntToStr(strgrData->Col)
		 + " ещё есть данные. Удалить?");

		switch (Application->MessageBox((L"В столбце " + IntToStr(strgrData->Col)
		 + " ещё есть данные. Удалить?").w_str(),
		Form1->Caption.w_str(), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
		{
			case IDYES:
				break;
			case IDNO:
				return;
		}
		}
		strgrData->ColWidths[1] = -1;//Чтобы не было видно этот столбик(т.к удаляем при 2-х столбиках)
		teYearCount->Text = 0;
		btEditDelete->Enabled = false;
		btRezCalculate->Enabled = false;
		btRezSaveAs->Enabled = false;
		strgrData->Options = strgrData->Options >> goEditing;//Включаю//выключаю опции
		strgrData->Options = strgrData->Options >> goColSizing;
		return;
	}

	if(strgrData->Cells[strgrData->Col][1] != L""){//Если столбиков > 2

		StatusBar->Panels->Items[0]->Text =(L"В столбце " + IntToStr(strgrData->Col)
		 + " ещё есть данные. Удалить?");

		switch (Application->MessageBox((L"В столбце " + IntToStr(strgrData->Col)
		 + " ещё есть данные. Удалить?").w_str(),
		Form1->Caption.w_str(), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
		{
			case IDYES:
				break;
			case IDNO:
				return;
		}
	}


	strgrData->Cols[strgrData->Col]->Clear();//Очищаю ячейку
	((TMyStringGrid*)strgrData)->DeleteColumn(strgrData->Col);//Обращаемся к функции удаления
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
	if(menuSave->Enabled == true)//Проверка, есть ли в таблице что-то на данный момент
	{
		int msg; //Переменная для вывода сообщения на панель
		if(StatusBar->Panels->Items[1]->Text == L""){
			StatusBar->Panels->Items[0]->Text =(L"Остались несохраненные данные!"
			" Сохранить их в новый файл?");

			msg = Application->MessageBox(L"Остались несохраненные данные!"
			" Сохранить их в новый файл?",
			L"Вычисление интенсивности движения", MB_YESNOCANCEL | MB_ICONQUESTION//Название самого окна
			| MB_DEFBUTTON3);
		}
		else
			msg = Application->MessageBox((L"Данные, которые загружены из файла, \""
			 + StatusBar->Panels->Items[1]->Text + "\" были изменены!" +
			 " Cохранить их в старый же файл?").w_str(),
			L"Вычисление интенсивности движения", MB_YESNOCANCEL | MB_ICONQUESTION
			| MB_DEFBUTTON3);

		switch(msg)
		{
			case ID_YES://Если пользователь захочет сохранить данные, вызвать кнопку сохранения
			{
				menuSave->Click();
			};
			case ID_NO://Ну нет, так нет
			{
				break;
			}
			case ID_CANCEL://Если нажал случайно
			{
				return;
			}
		}
	}

	if(!OpenFile->Execute())//Если не открылся файл
		return;

	fstream f((OpenFile->FileName).c_str(), ios::in);

	if(f.peek() == EOF)//Проверка на пустой файл
	{
		StatusBar->Panels->Items[1]->Text = L"Выбранный файл пуст.";
		Application->MessageBox(L"Выбранный файл пуст.", Form1->Caption.w_str(),
		 MB_OK | MB_ICONEXCLAMATION);
		f.close();
		return;
	}
	StatusBar->Panels->Items[1]->Text = OpenFile->FileName;//Выводим имя открывшегося файла

	int countPair = 0 ;//Переменная для кол-ва пар год и интенсивность
	try
	{
		f.read((char *)&countPair, sizeof(countPair));
		Data.Length = countPair;
		f.read((char*)&Data[0], sizeof(Data[0]) * countPair);

		if(f.peek() != EOF)//Проверка на некорректный файл
		{
			StatusBar->Panels->Items[1]->Text = L"Ошибка! Данные файла были повреждены.";
			Application->MessageBox(L"Ошибка! Данные файла были повреждены.",
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			f.close();//Закрываем файл
			return;
		}
	}
	catch(...)
	{
		StatusBar->Panels->Items[1]->Text = L"Ошибка чтения файла.";
		Application->MessageBox(L"Ошибка чтения файла.", Form1->Caption.w_str(),
		MB_OK | MB_ICONERROR);
		f.close();
		return;
	}
	f.close();//Закрываем
	strgrData->ColCount = countPair + 1; //Потому что один столбец фиксированный

	if(strgrData->ColWidths[1] == -1)//Если первый столбец скрыт
	{
		strgrData->ColWidths[1] = strgrData->DefaultColWidth;//Возвращаю стандартное значение
		strgrData->Options = strgrData->Options << goColSizing;
		if(cbEditingM->Checked)//Если стоит галочка на редактировании ячеек
			strgrData->Options = strgrData->Options << goEditing;
	}

	for(int i = 1; i < countPair + 1; i++)
	{
		strgrData->Cols[i]->Clear();//Сначала очищаю ячейку
		strgrData->Cells[i][0] = IntToStr(Data[i - 1].Year);//Вывожу год
		//(Data-1)- потому что начинать надо с 0, а в цикле начинаем с 1
		if(Data[i - 1].Intensity == -1)//Если ничего нет, то пропустить
			strgrData->Cells[i][1] = L" ";
		else
		strgrData->Cells[i][1] = IntToStr(Data[i - 1].Intensity);//Иначе заполнить
	}
		if(cbEditingM->Checked)
			cbEditingM->Checked = false;

    if(cbCorrectColumnWidth->Checked || menuCorrectColumnWidth->Checked )//Если стоит галочка
	{
		if(strgrData->ColWidths[1] == -1)//Если нет данных
			return;

		transformCells(strgrData);//Передаю в transformCells данный strgrData
	}
	teYearCount->Text = strgrData->ColCount-1;////
	btEditDelete->Enabled = true;
	menuNew->Enabled = true;//Ибо можно сделать не то действие, а дать возможность открыть нужно
	menuSave->Enabled = false;//Потому что только что открыли, зачем сохранять
	menuSaveAs->Enabled = true;
	btRezSaveAs->Enabled = false;
	btRezCalculate->Enabled = true;
}

//---------------------------------------------------------------------------
 void __fastcall TForm1::strgrDataSetEditText(TObject *Sender, int ACol, int ARow,
          const UnicodeString Value)
{
	if(!menuSave->Enabled && CCText != Value && !strgrData->EditorMode)//Если не активна кнопка сохранения и изменился текст в ячейке
	{
		menuSave->Enabled = true;
		menuSaveAs->Enabled = true;
        btRezSaveAs->Enabled = true;
		btRezCalculate->Enabled = true;
	}
	if(cbCorrectColumnWidth->Checked)//Если стоит галочка на корр-ку ширины
		strgrData->ColWidths[ACol] = widthCalcForStrgrData(strgrData, ACol);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::strgrDataSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)

{
	if(ARow > 1)
		strgrData->Options = strgrData->Options>>goEditing;//Включаю

	if(ARow == 1 && cbEditingM->Checked == true)//Выключаю
		strgrData->Options = strgrData->Options<<goEditing;

	int val;//Переменная для перевода из строки в переменную типа int
	if(!TryStrToInt(strgrData->Cells[strgrData->Col][strgrData->Row], val) || val <= 0 || val > 100000)
	{//Перевожу выбранную ячейку и проверяю
		if(val > 100000)//Если больше максимального числа, то вывожу в статусбар
		{
			StatusBar->Panels->Items[0]->Text = L"Ошибка! Значение интенсивности не должно"
			" превышать 100000";
			return;
		}
	}
//Если активна кнопка и включен режим редактирования и данные в ячейке изменены
//Или если включен режим редактирования и данные в ячейке изменены
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
		int toStrValue;//Переменная для преобразования знач-я строки в целочисленное
		//Если в введенных данных есть ошибка
		if( !TryStrToInt(strgrData->Cells[i][1], toStrValue) || toStrValue <= 0 || toStrValue > 100000 ||
		strgrData->Cells[i][1] == L"" || strgrData->ColCount <= 2)
		{
			strgrData->Col = i;//Идем по столбикам с годами
			strgrData->Row = 1;//Если пользователь выделил другую ячейку, в которой нет ошибки, его переведет в ту , где ошибка
			StatusBar->Panels->Items[0]->Text = L"Введено недостаточно данных или данные некорректны.";
			Application->MessageBox(L"Введено недостаточно данных или данные некорректны.",
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			return;
		}
	}
	aprGr->Series[0]->Clear();//Очищаю первый и второй графики для дальнейшего исп-я
	aprGr->Series[1]->Clear();

	//Если длина массива не равна кол-ву столбиков -1
	if(Data.Length != strgrData->ColCount - 1 || menuSave->Enabled == true )
	{
		Data.Length = strgrData->ColCount - 1;//То делаем это таковым
		for(int i = 0; i < Data.Length; i++)//От начала и до конца массива по порядку присваиваем значения(год и интенсивность)
		{
			Data[i].Year = StrToInt(strgrData->Cells[i + 1][0]);
			Data[i].Intensity = StrToInt(strgrData->Cells[i + 1][1]);
		}
	}
/*==============================================================================*/
	//Вычисление аппроксимированного закона изменения N(находим необходимое для нахождения a и b)
	int sumIntensity = 0, sumSquares = 0, sumYear = 0, sumPrIntensityYear = 0,
		prValSumSquares = 0, prIntensityYearOnPrYearIntensitu = 0 , i = 0, p = Data.Length;
	for(i = 0; i < p; i++)
	{
		sumYear = sumYear + (i+1);//Сумма годов(+1, т.к начинать надо с 1)
		sumIntensity += Data[i].Intensity;//Сумма интенсивностей
		sumSquares += pow(i + 1, 2);//Сумма квадратов(i+1 т.к начинать надо с 1^2)
		sumPrIntensityYear += Data[i].Intensity * (i + 1);//Сумма произведений интенсивности и года
	}
	prValSumSquares = p * sumSquares;//Произведение кол-ва интенсивностей на сумму квадратов
	prIntensityYearOnPrYearIntensitu = p * sumPrIntensityYear;//Произведение кол-ва пар (интенсивность+год) на сумму произведений интенсивности и года
/*==============================================================================*/
	float a = 0, b = 0; //Для вывода в мемо (по заданию a и b)
	a = (sumIntensity * sumSquares - sumYear * sumPrIntensityYear) / float(prValSumSquares - pow(sumYear, 2));
	b = (prIntensityYearOnPrYearIntensitu - sumYear * sumIntensity) / float(prValSumSquares - pow(sumYear, 2));
	//Использую приведение типов(к float), т.к. значения не целые

	//Вывод в доп. мемо формулу линейной аппроксимации с подставленным в нее численными значениями а и b
	if(b > 0)// b>0 - формула обычная
	{
		memLinearApr->Lines->Text = L"Формула:\r\n N = a + b * t = " + FloatToStrF(a,ffNumber,10,3)
		+ L" + " + FloatToStrF(b,ffNumber,10,3) + L"*t";
	}
	/*FloatToStrF - конвертирует значение с плавающей запятой в визуализуемую строку(из Delphi)
	  ffNumber(идет с FloatToStrF) - числовой формат, при выводе будет строка будет содержать разделители тысяч  */
	if(b == 0)// b=0 - формула без b
	{
		memLinearApr->Lines->Text = L"Формула N:\r\n N = a = " + FloatToStrF(a,ffNumber,10,3);
	}

	if(b < 0)// b<0 - плюс можно не писать(т.к минус)
	{
		memLinearApr->Lines->Text = L"Формула:\r\n N = a + b * t = " + FloatToStrF(a,ffNumber,10,3)
	   + L' - '	+ FloatToStrF(b,ffNumber,10,3) + L"*t";
	}

	DynamicArray <int> Appr;//Создаю динамический массив для хранения апроксимированных значений(целочисленный)
	Appr.Length = p; //Присваиваю Appr длину Data.Length
	for(i = 0; i < p; i++)
	{
		Appr[i] = a + b * (i + 1);//Заполняю динамический массив NAppr по формуле
		strgrData->Cells[i + 1][2] = IntToStr(Appr[i]);//Вывожу полученные значения во вторую строку, начиная с первого года
	}
/*==============================================================================*/
	//Расчет абсолютного и относительного прироста интенсивности
	int absIntensity[p];				//Создаю обычный массив для значений абсолютного прироста
	double relativeIntensity[p];		//Создаю обычный массив для значений относительного прироста
	for(i = 0; i < p; i++)
	{
		if(i == p - 1)
			break;
		//(m - 1)Потому что дальше я работаю с массивами, в нём всё начинается с 0, а Data.Length вернет длину без учета 0, как начала массива
		absIntensity[i] = Data[i + 1].Intensity - Data[i].Intensity;//Поочередно высчитываю среднюю интенсивность
		relativeIntensity[i] = (absIntensity[i] / float(Data[i + 1].Intensity)) * 100;//Приведение типов int к float
		strgrData->Cells[i + 2][3] = IntToStr(absIntensity[i]);//Конвертирую среднюю интенсивность в целочисленное значение и присваиваю соотв-й ячейке
		strgrData->Cells[i + 2][4] = FloatToStrF(relativeIntensity[i], ffNumber, 10, 1) + L"%";//Конвертирую значение с плавающей запятой в строку
	}
	strgrData->Cells[1][3] = L"0";//Для первого столбика высчитать невозможно , поэтому ставим им нули
	strgrData->Cells[1][4] = L"0";
/*==============================================================================*/
   //Расчет средней интенсивности
	float avgIntensity = 0, AvgAppr = 0;
	for(i = 0; i < p; i++)
	{
		avgIntensity += Data[i].Intensity;//Нахожу сумму интенсивностей
		AvgAppr += Appr[i];//Нахожу сумму аппроксимаций
	}
	avgIntensity = avgIntensity / p;//Нахожу среднее значение интенсивности
	AvgAppr = AvgAppr / p;  //Нахожу среднее значение интенсивности аппроксимации
/*==============================================================================*/
	//Расчет среднеквадратичного разброса интенсивности(avgSpreadIntensity)
	float squaredDeviation[p], dispersion = 0, avgSpreadIntensity, avgSpreadAppr;
	for(i = 0; i < p; i++)
	{
		squaredDeviation[i] = pow(Data[i].Intensity - avgIntensity, 2);
		dispersion += squaredDeviation[i];
	}
	dispersion = dispersion / p;
	avgSpreadIntensity = sqrt(dispersion);
	// Расчет среднеквадратичного разброса интенсивности для аппроксимированных значений(avgSpreadAppr)
	for(i = 0; i < p; i++)
	{
		squaredDeviation[i] = pow(Appr[i] - AvgAppr, 2);
		dispersion += squaredDeviation[i];
	}
	dispersion = dispersion / p;
	avgSpreadAppr = sqrt(dispersion);
	memLinearApr->Lines->Add(L"Среднеквадратичный разброс интенсивности N: "
	 + FloatToStrF(avgSpreadAppr, ffNumber, 10, 3));//Вывожу в спец-е memo

	 strgrData->Cells[1][7] = FloatToStrF(avgSpreadAppr, ffNumber, 10, 3);//Скрыто, но есть для того, чтобы записать в файл при сохранении
/*==============================================================================*/
	//Расчет линейного коэффициента корреляции между исходными и полученными после вычислений данными
	float sumPrOtklIn = 0;
	double linCoefCorr;
	for(i = 0; i < p; i++)
		sumPrOtklIn += (Data[i].Intensity - avgIntensity)*(Appr[i] - AvgAppr);
	if(avgSpreadAppr == 0 || avgSpreadIntensity == 0)
		linCoefCorr = 0;  //Потому что получится деление на 0
	else
		linCoefCorr = sumPrOtklIn /(p * avgSpreadAppr * avgSpreadIntensity);

	memLinearApr->Lines->Add(L"Коэффициент линейной корреляции: "
	+ FloatToStrF(linCoefCorr, ffNumber, 10, 3));

	strgrData->Cells[1][6] = FloatToStrF(linCoefCorr, ffNumber, 10, 3);//Скрыто, но есть для того, чтобы записать в файл при сохранении
/*==============================================================================*/
	//Расчет среднеквадратичной ошибки линейной аппроксимации в процентах от среднего значения интенсивности
	float misLinAppr;
	Appr[0]=1;
	for(i = 0; i < p; i++){
		misLinAppr = abs(Appr[i] - avgIntensity)/(Appr[i] * p) * 100;
		strgrData->Cells[i+1][5] = FloatToStrF(misLinAppr, ffNumber, 10, 1) + L"%";
	}

	memLinearApr->Lines->Add(L"Ошибка линейной аппроксимации: " +
	FloatToStrF(misLinAppr, ffNumber, 10, 1) + L"%");

	strgrData->Cells[1][5] = L"0";//Т.к в первом ноль

	// График теоретических значений интенсивности и эмпирических значений интенсивностей
	for(i = 0; i < p; i++)
	{
		aprGr->Series[0]->Add(Data[i].Intensity, i + 1);
		aprGr->Series[1]->Add(Appr[i], i + 1);
	}
    if(cbCorrectColumnWidth->Checked || menuCorrectColumnWidth->Checked )//Если стоит галочка
	{
		if(strgrData->ColWidths[1] == -1)//Если нет данных
			return;

		transformCells(strgrData);//Передаю в transformCells данный strgrData
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
	if(cbCorrectColumnWidth->Checked || menuCorrectColumnWidth->Checked )//Если стоит галочка
	{
		if(strgrData->ColWidths[1] == -1)//Если нет данных
			return;

		transformCells(strgrData);//Передаю в transformCells данный strgrData
	}

}
//---------------------------------------------------------------------------

void __fastcall TForm1::cbEditingMClick(TObject *Sender)
{
	if(cbEditingM->Checked == false)//Если галочки нет, то включаем и наоборот
		strgrData->Options = strgrData->Options >> goEditing;
	else
		strgrData->Options = strgrData->Options << goEditing;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::menuExitClick(TObject *Sender)
{
	Form1->Close();//При нажатии на кнопку, просто закрываем форму
}
//---------------------------------------------------------------------------

void __fastcall TForm1::menuNewClick(TObject *Sender)
{
	if(menuSave->Enabled == true)//Если кнопка сохранения активна(есть какие-то данные)
	{
		int msg;//Снова переменная для вывода окна
		if(StatusBar->Panels->Items[1]->Text == L""){
			msg = Application->MessageBox(L"Остались несохраненные данные!"
			" Сохранить их в новый файл?",
			L"Вычисление интенсивности движения", MB_YESNOCANCEL | MB_ICONQUESTION//Название самого окна
			| MB_DEFBUTTON3);
		}
		else
			msg = Application->MessageBox((L"Данные, которые загружены из файла, \""
			 + StatusBar->Panels->Items[1]->Text + "\" были изменены!" +
			 " Cохранить их в старый же файл?").w_str(),
			L"Вычисление интенсивности движения", MB_YESNOCANCEL | MB_ICONQUESTION
			| MB_DEFBUTTON3);
		switch(msg)
		{
			case ID_YES://Если пользователь захочет сохранить, то сохранить(вызвать другую кнопку)
			{
				menuSave->Click();//Вызов кнопки сохранения
			};
			case ID_NO://Не хочет, так не хочет...
			{
				break;
			}
			case ID_CANCEL:
			{
				Abort();//Аварийное завершение
			}
		}
	}

	//Очистка таблицов(абсолютно всё с чистого листа)
	for(int i = 1; i < strgrData->ColCount; i++)
	{
		strgrData->Cols[i]->Clear();//Очищаю каждый столбик
		strgrData->Cells[i][0] = IntToStr(i);//Расставляю в правильном порядке номера столбиков
	}
	strgrData->ColCount = 2;//Оставляю только 2 столбика(один для данных, другой для значений)
	strgrData->ColWidths[1] = -1;//Скрываю один
	strgrData->Options = strgrData->Options >> goEditing;//Включаю возможность редактирования
	strgrData->Options = strgrData->Options >> goColSizing;//Включаю возможность изменения ширины

	teYearCount->Clear();//Очищаю поле для ввода кол-ва лет
	StatusBar->Panels->Items[1]->Text = L"";//Очищаю статусбар
	StatusBar->Panels->Items[0]->Text = L"";
	memLinearApr->Lines->Text = L"";//Полностью очищаю дополнительное мемо
	Data.Length = 0;//Обнуляю длину, ибо сохраняются прошлые, что вызывает неправильную работу
	aprGr->Series[0]->Clear();
	aprGr->Series[1]->Clear();
	if(!cbEditingM->Checked)//Если кнопка редактирования не активна, активировать
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
			//Изменение ширины фиксированного столбца
			strgrData->ColWidths[0] = strgrData->Canvas->TextWidth(strgrData->Cells[0][4]) + 8;

			//Изменение ширины столбцов
			if(strgrData->ColWidths[1] != -1)
				transformCells(strgrData);

			//Изменение высоты строк
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
   if(FromIndex < ToIndex)//Если при переносе индекс(год) переносимой строки меньше чем тот, в который переносим
		for(int i = ToIndex; i >= 1; i--)//Перенумеровываем двигаясь вниз(пока номер года не будет равен 1)
			strgrData->Cells[i][0] = i; //Сам процесс перенумеровывания
	else
		for(int i = ToIndex; i <= FromIndex; i++)//Если индекс(год) больше, чем тот, в который переносим
			strgrData->Cells[i][0] = i;//Сам процесс перенумеровывания

		if(strgrData->Cells[FromIndex][1] != L"" || strgrData->Cells[ToIndex][1] != L"")//Активировать кнопку сохранения после переноса
		{
			menuSave->Enabled = true;
		}
     btRezCalculate->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::strgrDataDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect,
		  TGridDrawState State)
{
	//Закраска всех ячеек с полученными при расчетах данными
	if(ARow > 1 && ARow < 6 && ACol > 0)//От 1 до 6 (строчки) и от 0 столбца
	{
		Rect.Left -= 4;
		strgrData->Canvas->Brush->Color = clWebSilver;//Указываю серый цвет
		strgrData->Canvas->FillRect(Rect);
		strgrData->Canvas->TextOut(Rect.left + 6, Rect.Top + 2,
		strgrData->Cells[ACol][ARow]);
	}
	//Закраска некорректных и не пустых значений ячейки
	if(ARow == 1 && ACol > 0 && strgrData->Cells[ACol][ARow] != L"")//С первой строчки и первого столбика вправо(+если там вообще есть  значения)
	{
		int toStrValue;
		if(!TryStrToInt(strgrData->Cells[ACol][ARow], toStrValue) || toStrValue <= 0 ||
		toStrValue > 100000)//Если введенные данные некорректны, то крашу в красный
		{
			Rect.Left -= 4;
			strgrData->Canvas->Brush->Color = clWebLightCoral;//Указываю красный цвет
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
			StatusBar->Panels->Items[0]->Text = L"Ошибка! Введен недопустимый символ '" +
			UnicodeString(Key) + L"'";
			Key = 0;
			return;
		}
		else StatusBar->Panels->Items[0]->Text = L"";//Иначе очистить

		if(Key == VK_ESCAPE && strgrData->EditorMode)//Если пользователь нажмет esc, то отменяю действия
		strgrData->Cells[strgrData->Col][strgrData->Row] = CCText;

			if(Key == VK_RETURN && strgrData->EditorMode)
	{
		int p;//Переменная для перевода
		if(!TryStrToInt(strgrData->Cells[strgrData->Col][strgrData->Row], p) || p <= 0 || p > 100000)
		{
			if(p > 100000)
			{
				StatusBar->Panels->Items[0]->Text = L"Ошибка! Максимальное значение интенсивности: 100000";
				return;
			}
			if(strgrData->Cells[strgrData->Col][strgrData->Row] == L"")
				return;//Если строка полностью удалена
			StatusBar->Panels->Items[0]->Text = L"Ошибка! Некорректное значение: " +
			strgrData->Cells[strgrData->Col][strgrData->Row];
			return;
		}

		if((strgrData->Col + 1) != strgrData->ColCount)
			strgrData->Col += 1;//При нажатии на Enter перевод фокуса ввода на следующую ячейку

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
				StatusBar->Panels->Items[0]->Text = L"Ошибка! Введен недопустимый символ ''" +
				UnicodeString(Key) + L"''";
				Key = 0;
				return;
			}
			else StatusBar->Panels->Items[0]->Text = L"";//Иначе очистить
}
//---------------------------------------------------------------------------

void __fastcall TForm1::strgrDataGetEditText(TObject *Sender, int ACol, int ARow,
          UnicodeString &Value)
{
	CCText = Value; //Если отменяю изменение, то оставить прошлое значение
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TimerTimer(TObject *Sender)
{
	StatusBar->Panels->Items[0]->Text = L"";
}

//---------------------------------------------------------------------------

void __fastcall TForm1::menuSaveValClick(TObject *Sender)
{   //Сохраняю всю таблицу с первой ячейки и до последней
	//Фильтр для текстового формата
	SaveFile->Filter = L"Текстовый файл (*.txt)|*.txt|Все файлы (*.*)|*.*";
	SaveFile->DefaultExt = L"txt";

	if(!SaveFile->Execute())//Если не получается сохранить
		return;
	ofstream f;//Переменная для файла
	f.open((SaveFile->FileName).c_str(), ios::out);
	if(!f.is_open())//Если не получается открыть
	{
		Application->MessageBox(L"Не удалось получить доступ к файлу",
		 Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
		return;
	}

	for(int i = 0; i < strgrData->RowCount + 2; i++)//+2 т.к. вывожу ещё доп информацию(а этих 2-х строчек нет в таблице)
	{
		f << setw(35) << left << AnsiString(strgrData->Cells[0][i]).c_str() << ':';//Сохраняю + делаю отступ
		for(int j = 1; j < strgrData->ColCount; j++)
		{
			if(strgrData->Cells[j][i] == L"")//Если ничего нет в ячейке, всё равно сохраняю
				continue;
			f << setw(10) << right << AnsiString(strgrData->Cells[j][i]).c_str();//Выравнивание по правой стоороне значений и отступ
		}
		f << endl;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::menuSaveGraphClick(TObject *Sender)
{
	SaveFile->Filter = "Файл изображения Bitmap (*.bmp)|*.bmp|Файл изображения MetaFile (*.emf)|*.emf|Все файлы (*.*)|*.*";

	if(!SaveFile->Execute())//Если возникла ошибка сохранения
		return;

	//Установка фильтра по расширению, написанного в окне сохранения
	String format;//Переменная для формата
	format = SaveFile->FileName;
	for(int i = 1; i <= format.Length(); i++)//Если после точки стоит b, то фильтр bmp и т.д
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

	if(strgrData->ColWidths[1] == -1)// Изменение ширины 1-го столбика
	{
		teYearCount->Text = strgrData->ColCount-1;
		strgrData->ColWidths[1] = strgrData->DefaultColWidth;//Дефолтное значение возвращаю
		if(cbEditingM->Checked)//Если стоит галочка на режиме редактирования
			strgrData->Options = strgrData->Options<<goEditing;
		strgrData->Options = strgrData->Options << goColSizing;
	}
	else
	{
		strgrData->ColCount++;//Вставить столбик
		((TMyStringGrid*)strgrData)->MoveColumn(strgrData->ColCount - 1, strgrData->Col);//Подвигаю вправо
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
			mVal = Application->MessageBox(L"Остался несохраненный набор данных. "
			"Сохранить в новый файл?",
			Form1->Caption.w_str(), MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON3);
		else
			mVal = Application->MessageBox((L"Данные, загруженные из файла\"" +
			 StatusBar->Panels->Items[0]->Text +
			L"\" были изменены. Хотите ли вы сохранить их в тот тже файл?").w_str(),
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


	SaveFile->Filter = L"Текстовый файл (*.txt)|*.txt|Все файлы (*.*)|*.*";
	SaveFile->DefaultExt = L"txt";

	if(!SaveFile->Execute())
		return;
	ofstream f;
	f.open((SaveFile->FileName).c_str(), ios::out);
	if(!f.is_open())
	{
		Application->MessageBox(L"Не удалось получить доступ к файлу",
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
	for(int i = 1; i < strgrData->ColCount; i++)//От 1 до кол-ва столбиков
	{
		if(strgrData->Cells[i][1] == L"")//Пустые ячейки сохраняю
			continue;
		int toStrValue;//Переменная для преобразования знач-я строки в целочисленное
		if(!TryStrToInt(strgrData->Cells[i][1], toStrValue) || toStrValue <= 0)//Если число не целочисленное, ошибка
		{
			strgrData->Col = i;
			strgrData->Row = 1; //Вдруг выделена ячейка с инедексом, который не 1
			StatusBar->Panels->Items[0]->Text = (L"Ошибка! Обнаружены некорректные данные в столбце "
			+ IntToStr(i));
			Application->MessageBox((L"Ошибка! Обнаружены некорректные данные в столбце "
			+ IntToStr(i)).w_str(),
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);//Вывод предупреждения
			Abort();
		}
	}
	//Задаю фильтр
	SaveFile->Filter = L"Файл с данными интенсивности (*.msw)|*.msw|Все файлы (*.*)|*.*";

	if(StatusBar->Panels->Items[1]->Text == "")//Если в статус баре ничего нет
	{
		if(!SaveFile->Execute())
			Abort();//Если не удалось выполнить, то закроет программу
			//Abort()-аварийно завершает работу программы, генерируя сигнал, который вызывает завершение программы
		StatusBar->Panels->Items[1]->Text = SaveFile->FileName;//Вывод имени файла и его расположения в статусбар
	}

	if(menuSave->Enabled)//Если кнопка сохранения доступна
	{
		Data.Length = strgrData->ColCount - 1;//Потому что один скрыт и не относится к сохр-мым данным
		for(int i = 0; i < Data.Length; i++)//От 0 до длинны Data
		{
			if(strgrData->Cells[i + 1][1] == L"" && i != Data.Length)//Если первый столбик пуст
				Data[i].Intensity = -1;
			else
			Data[i].Intensity = StrToInt(strgrData->Cells[i + 1][1]);//Если всё хорошо, то заполняю интенсивность
			Data[i].Year = StrToInt(strgrData->Cells[i + 1][0]);//Если всё хорошо, то заполняю
		}
	}

	switch(DataSave(StatusBar->Panels->Items[1]->Text, Data))
	{
		case allGood: break;
		case fileOpenError:
		{
			Application->MessageBox(L"Ошибка открытия файла для записи данных.",
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			return;
		}
		case lackOfMemoryErr:
		{
			Application->MessageBox(L"Недостаточно места для сохранения файла!",
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			return;
		}
	}

	menuSave->Enabled = false;//Отключаю кнопку, ибо только что сохранил
	menuSaveAs->Enabled = true;
	btRezSaveAs->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::menuSaveAsClick(TObject *Sender)
{  //Почти тоже самое, что и сохранить
	for(int i = 1; i < strgrData->ColCount; i++)
	{
		if(strgrData->Cells[i][1] == L"") //т.к. Пустые ячейки тоже нужно сохранять
			continue;
		int nval;
		if(!TryStrToInt(strgrData->Cells[i][1], nval) || nval<=0)
		{
			strgrData->Col = i;
			strgrData->Row = 1; // Если выделена ячейка строки с инедексом, не равным 1
			Application->MessageBox((L"Ошибка! Обнаружено некорректное значение в столбце №"
			 + IntToStr(i)).w_str(),
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			Abort();
		}
	}

	SaveFile->Filter = L"Файл с данными интенсивности (*.msw)|*.msw|Все файлы (*.*)|*.*";
	StatusBar->Panels->Items[1]->Text = L"";

	if(StatusBar->Panels->Items[1]->Text == "")//Если в статус баре ничего нет
	{
		if(!SaveFile->Execute())
			Abort();//Если не удалось выполнить, то закроет программу
			//Abort()-аварийно завершает работу программы, генерируя сигнал, который вызывает завершение программы
		StatusBar->Panels->Items[1]->Text = SaveFile->FileName;//Вывод имени файла и его расположения в статусбар
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
			Application->MessageBox(L"Ошибка открытия файла для записи данных.",
			Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
			return;
		}
		case lackOfMemoryErr:
		{
			Application->MessageBox(L"Ошибка! Недостаточно места для сохранения"
			" данных", Form1->Caption.w_str(), MB_OK | MB_ICONERROR);
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
	GetCursorPos(&mousePos);    //Положение курсора в экранных координатах
	mousePos = (strgrData->ScreenToClient(mousePos));   //Преобразование экранных координат в клиентские
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
			int i = abs(rowNmbAndSortKind) - 1; // Индекс строки

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

			strgrData->Cols[strgrData->Col]->Objects[0] = (TObject*)true;//Установка признака фокуса на столбце на котором находится фокус

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

