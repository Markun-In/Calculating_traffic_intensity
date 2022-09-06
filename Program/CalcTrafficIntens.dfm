object Form1: TForm1
  Left = 0
  Top = 0
  Caption = #1042#1099#1095#1080#1089#1083#1077#1085#1080#1077' '#1080#1085#1090#1077#1085#1089#1080#1074#1085#1086#1089#1090#1080' '#1076#1074#1080#1078#1077#1085#1080#1103
  ClientHeight = 497
  ClientWidth = 849
  Color = clBtnFace
  Constraints.MinHeight = 318
  Constraints.MinWidth = 562
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -13
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnCloseQuery = FormCloseQuery
  OnCreate = FormCreate
  DesignSize = (
    849
    497)
  PixelsPerInch = 120
  TextHeight = 16
  object lYearCount: TLabel
    Left = 8
    Top = 21
    Width = 97
    Height = 16
    Caption = #1050#1086#1083#1080#1095#1077#1089#1090#1074#1086' '#1083#1077#1090':'
  end
  object btYearCount: TButton
    Left = 207
    Top = 18
    Width = 90
    Height = 24
    Caption = #1047#1072#1076#1072#1090#1100
    TabOrder = 1
    OnClick = btYearCountClick
  end
  object gbRed: TGroupBox
    Left = 315
    Top = 1
    Width = 241
    Height = 49
    Caption = #1056#1077#1076#1072#1082#1090#1080#1088#1086#1074#1072#1085#1080#1077':'
    TabOrder = 2
    object btEditAdd: TButton
      Left = 3
      Top = 17
      Width = 75
      Height = 25
      Caption = #1044#1086#1073#1072#1074#1080#1090#1100
      TabOrder = 0
      OnClick = btEditAddClick
    end
    object btEditDelete: TButton
      Left = 161
      Top = 17
      Width = 75
      Height = 25
      Caption = #1059#1076#1072#1083#1080#1090#1100
      Enabled = False
      TabOrder = 2
      OnClick = btEditDeleteClick
    end
    object btEditInsert: TButton
      Left = 84
      Top = 17
      Width = 75
      Height = 25
      Caption = #1042#1089#1090#1072#1074#1080#1090#1100
      TabOrder = 1
      OnClick = btEditInsertClick
    end
  end
  object gbRez: TGroupBox
    Left = 559
    Top = 1
    Width = 268
    Height = 49
    Caption = #1056#1077#1079#1091#1083#1100#1090#1072#1090#1099' '#1086#1073#1088#1072#1073#1086#1090#1082#1080':'
    TabOrder = 4
    object btRezCalculate: TButton
      Left = 3
      Top = 17
      Width = 110
      Height = 25
      Caption = #1056#1072#1089#1089#1095#1080#1090#1072#1090#1100
      Enabled = False
      TabOrder = 0
      OnClick = btRezCalculateClick
    end
    object btRezSaveAs: TButton
      Left = 119
      Top = 17
      Width = 130
      Height = 25
      Hint = #1057#1086#1093#1088#1072#1085#1080#1090#1100' '#1095#1080#1089#1083#1086#1074#1099#1077' '#1079#1085#1072#1095#1077#1085#1080#1103' '#1082#1072#1082'... Ctrl+G'
      Caption = #1057#1086#1093#1088#1072#1085#1080#1090#1100' '#1082#1072#1082'...'
      Enabled = False
      ParentShowHint = False
      ShowHint = True
      TabOrder = 1
      OnClick = btRezSaveAsClick
    end
  end
  object cbEditingM: TCheckBox
    Left = 321
    Top = 49
    Width = 232
    Height = 17
    Caption = #1056#1077#1076#1072#1082#1090#1080#1088#1086#1074#1072#1085#1080#1077' '#1076#1072#1085#1085#1099#1093' '#1074' '#1103#1095#1077#1081#1082#1072#1093
    Checked = True
    State = cbChecked
    TabOrder = 5
    OnClick = cbEditingMClick
  end
  object cbCorrectColumnWidth: TCheckBox
    Left = 562
    Top = 49
    Width = 226
    Height = 17
    Caption = #1050#1086#1088#1088#1077#1082#1090#1080#1088#1086#1074#1082#1072' '#1096#1080#1088#1080#1085#1099' '#1089#1090#1086#1083#1073#1094#1086#1074
    TabOrder = 6
    OnClick = cbCorrectColumnWidthClick
  end
  object memLinearApr: TMemo
    Left = 555
    Top = 240
    Width = 276
    Height = 234
    Anchors = [akTop, akRight, akBottom]
    HideSelection = False
    Lines.Strings = (
      'memLinearApr')
    ReadOnly = True
    TabOrder = 7
  end
  object strgrData: TStringGrid
    Left = -4
    Top = 72
    Width = 835
    Height = 169
    Anchors = [akLeft, akTop, akRight]
    BevelInner = bvNone
    BevelOuter = bvNone
    ColCount = 2
    FixedColor = clWhite
    RowCount = 6
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goColSizing, goColMoving, goEditing, goTabs, goThumbTracking, goFixedColClick]
    ParentFont = False
    TabOrder = 3
    OnColumnMoved = strgrDataColumnMoved
    OnDblClick = strgrDataDblClick
    OnDrawCell = strgrDataDrawCell
    OnGetEditText = strgrDataGetEditText
    OnKeyPress = strgrDataKeyPress
    OnSelectCell = strgrDataSelectCell
    OnSetEditText = strgrDataSetEditText
  end
  object StatusBar: TStatusBar
    Left = 0
    Top = 478
    Width = 849
    Height = 19
    Panels = <
      item
        Width = 300
      end
      item
        Width = 300
      end>
  end
  object aprGr: TChart
    Left = 0
    Top = 240
    Width = 557
    Height = 233
    Legend.Alignment = laBottom
    Legend.CheckBoxes = True
    Title.Text.Strings = (
      '')
    Title.Visible = False
    BottomAxis.Title.Caption = #1043#1086#1076#1099
    LeftAxis.Title.Caption = #1047#1085#1072#1095#1077#1085#1080#1103' '#1080#1085#1090#1077#1085#1089#1080#1074#1085#1086#1089#1090#1080' N, '#1072#1074#1090#1086'/'#1089#1091#1090#1082#1080
    View3D = False
    View3DOptions.Zoom = 106
    TabOrder = 9
    Anchors = [akLeft, akTop, akRight, akBottom]
    DefaultCanvas = 'TGDIPlusCanvas'
    ColorPaletteIndex = 13
    object Series1: TLineSeries
      SeriesColor = clMaroon
      Title = #1058#1077#1086#1088#1077#1090#1080#1095#1077#1089#1082#1080#1077' '#1079#1085#1072#1095#1077#1085#1080#1103' N'
      Brush.BackColor = clDefault
      Dark3D = False
      LinePen.Width = 2
      Pointer.Draw3D = False
      Pointer.HorizSize = 3
      Pointer.InflateMargins = True
      Pointer.Style = psCircle
      Pointer.VertSize = 3
      Pointer.Visible = True
      PointerBehind = True
      XValues.Name = 'X'
      XValues.Order = loAscending
      YValues.Name = 'Y'
      YValues.Order = loNone
    end
    object Series2: TLineSeries
      Title = #1069#1084#1087#1080#1088#1080#1095#1077#1089#1082#1080#1077'('#1072#1087#1087#1088'.) '#1079#1085#1072#1095#1077#1085#1080#1103' N'
      Brush.BackColor = clDefault
      LinePen.Width = 2
      Pointer.Draw3D = False
      Pointer.HorizSize = 3
      Pointer.InflateMargins = True
      Pointer.Style = psCircle
      Pointer.VertSize = 3
      Pointer.Visible = True
      PointerBehind = True
      XValues.Name = 'X'
      XValues.Order = loAscending
      YValues.Name = 'Y'
      YValues.Order = loNone
    end
  end
  object teYearCount: TEdit
    Left = 111
    Top = 18
    Width = 90
    Height = 24
    NumbersOnly = True
    TabOrder = 0
    OnKeyPress = teYearCountKeyPress
  end
  object MainMenu1: TMainMenu
    Left = 584
    Top = 88
    object mFileDialog: TMenuItem
      Caption = #1060#1072#1081#1083
      object menuNew: TMenuItem
        Caption = #1057#1086#1079#1076#1072#1090#1100' '#1085#1086#1074#1099#1081
        ShortCut = 16462
        OnClick = menuNewClick
      end
      object menuOpenF: TMenuItem
        Caption = #1054#1090#1082#1088#1099#1090#1100'...'
        ShortCut = 16463
        OnClick = menuOpenFClick
      end
      object menuSave: TMenuItem
        Caption = #1057#1086#1093#1088#1072#1085#1080#1090#1100
        Enabled = False
        ShortCut = 16467
        OnClick = menuSaveClick
      end
      object menuSaveAs: TMenuItem
        Caption = #1057#1086#1093#1088#1072#1085#1080#1090#1100' '#1082#1072#1082'...'
        Enabled = False
        ShortCut = 49235
        OnClick = menuSaveAsClick
      end
      object menuSeparator: TMenuItem
        Caption = '-'
        Enabled = False
      end
      object menuExit: TMenuItem
        Caption = #1042#1099#1093#1086#1076
        OnClick = menuExitClick
      end
    end
    object mFileRed: TMenuItem
      Caption = #1056#1077#1076#1072#1082#1090#1080#1088#1086#1074#1072#1085#1080#1077
      object menuColAdd: TMenuItem
        Caption = #1044#1086#1073#1072#1074#1080#1090#1100
        ShortCut = 16449
        OnClick = btEditAddClick
      end
      object menuColIns: TMenuItem
        Caption = #1042#1089#1090#1072#1074#1080#1090#1100
        ShortCut = 49238
      end
      object menuDelete: TMenuItem
        Caption = #1059#1076#1072#1083#1080#1090#1100
        ShortCut = 16452
        OnClick = btEditDeleteClick
      end
    end
    object menuResults: TMenuItem
      Caption = #1056#1077#1079#1091#1083#1100#1090#1072#1090#1099
      object menuRezCalculate: TMenuItem
        Caption = #1056#1072#1089#1089#1095#1080#1090#1072#1090#1100' '#1088#1077#1079#1091#1083#1100#1090#1072#1090
        Enabled = False
        ShortCut = 16466
        OnClick = btRezCalculateClick
      end
      object menuSaveVal: TMenuItem
        Caption = #1057#1086#1093#1088#1072#1085#1080#1090#1100' '#1095#1080#1089#1083#1086#1074#1099#1077' '#1079#1085#1072#1095#1077#1085#1080#1103' '#1082#1072#1082'...'
        ShortCut = 16455
        OnClick = menuSaveValClick
      end
      object menuSaveGraph: TMenuItem
        Caption = #1057#1086#1093#1088#1072#1085#1080#1090#1100' '#1075#1088#1072#1092#1080#1082' '#1082#1072#1082'...'
        ShortCut = 16464
        OnClick = menuSaveGraphClick
      end
    end
    object menuView: TMenuItem
      Caption = #1042#1080#1076
      object menuEditFont: TMenuItem
        Caption = #1064#1088#1080#1092#1090'...'
        ShortCut = 16454
        OnClick = menuEditFontClick
      end
      object menuCorrectColumnWidth: TMenuItem
        AutoCheck = True
        Caption = #1050#1086#1088#1088#1077#1082#1090#1080#1088#1086#1074#1082#1072' '#1096#1080#1088#1080#1085#1099' '#1089#1090#1086#1083#1073#1094#1086#1074
        OnClick = cbCorrectColumnWidthClick
      end
    end
  end
  object FontDialog1: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    Left = 656
    Top = 88
  end
  object OpenFile: TOpenDialog
    OnShow = OpenFileShow
    DefaultExt = 'msw'
    Filter = #1048#1085#1090#1077#1085#1089#1080#1074#1085#1086#1089#1090#1100' '#1076#1074#1080#1078#1077#1085#1080#1103' '#1040#1058' (*.msw)|*.msw|'#1042#1089#1077' '#1092#1072#1081#1083#1099' (*.*)|*.*'
    Options = [ofHideReadOnly, ofFileMustExist, ofEnableSizing]
    Left = 520
    Top = 88
  end
  object SaveFile: TSaveDialog
    DefaultExt = 'bin'
    Filter = '*.txt|*.txt'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Left = 456
    Top = 88
  end
  object Timer: TTimer
    Interval = 6000
    OnTimer = TimerTimer
    Left = 720
    Top = 88
  end
end
