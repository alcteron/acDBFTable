using System;
using System.Text;
using System.IO;
using System.Globalization;
using System.Linq;

// ReSharper disable CheckNamespace
namespace DBFTable
// ReSharper restore CheckNamespace
{
	//использется для представления дат.
    public struct DBFDate
    {
        public ushort Year, Month, Day;
		
		//конструктор
        public DBFDate(ushort y, ushort m, ushort d)
        {
            Year = y;
            Month = m;
            Day = d;
        }
		//перегруженный метод. Конвертирует дату в строковое представление 
		//(по умолчанию, разделителем является символ "/"
        public override string ToString()
        {
            return string.Format("{0}/{1}/{2}", Year.ToString().Substring(0, 4), Month.ToString().Substring(0, 2), Day.ToString().Substring(0, 2));
        }
		//то же, но с произвольно задаваемым разделителем
        public string ToString( char dateDecimator )
        {
            return string.Format("{0}{1}{2}{3}{4}",
                                 Year.ToString().Substring(0, 4),
                                 dateDecimator,
                                 Month.ToString().Substring(0, 2),
                                 dateDecimator,
                                 Day.ToString().Substring(0, 2));
        }

		//Метод сравнивает текущую дату с заданной в аргументе
        //Возвращает 0 если даты равны,
        //          -1 если текущая дата раньше date2 (передаваемого аргумента),
        //           1 если текущая дата  позже date2 (передаваемого аргумента).
        public sbyte CompareTo( DBFDate date2 )
        {
            if( Year < date2.Year ) return -1;
            if( Year > date2.Year ) return 1;
            if( Month < date2.Month ) return -1;
            if( Month > date2.Month ) return 1;
            if( Day < date2.Day ) return -1;
            if( Day > date2.Day ) return 1;
            return 0;
        }
    };

	//Реализация структуры, описывающей поле в заголовке dBase-таблицы
// ReSharper disable InconsistentNaming
    public struct DBFFieldDescriptor

    {
        public byte[] strName;		//11 bytes
    	public byte bFieldType;		//01 byte
	    public uint dwAddress;		//04 bytes
    	public byte bFieldSize;		//01 byte
	    public byte bFractionalSize;	//01 byte
    	public ushort wReserved;			//02 bytes
	    public byte bWorkAreaID;		//01 byte
    	public ushort wMutiUser;			//02 bytes
	    public byte bSetFields;        //01 byte
    	public byte[] baReserved;		//07 bytes
	    public byte bMDXIDIncluded;		//01 byte
        public DBFHeader Parent;
// ReSharper restore InconsistentNaming
        
        private ushort _id;

		//Конструктор
        public DBFFieldDescriptor(DBFHeader parent, ushort id)
        {
            Parent = parent;
            _id = id;
            strName = new byte[11];
            bFieldType = 0;
            dwAddress = 0;
            bFieldSize = 0;
            bFractionalSize = 0;
            wReserved = 0;
            bWorkAreaID = 0;
            wMutiUser = 0;
            bSetFields = 0;
            baReserved = new byte[2];
            bMDXIDIncluded = 0;
        }
    };

	//Реализация структуры, описывающей заголовок dBase-таблицы
// ReSharper disable InconsistentNaming
	public class DBFHeader
	{
		public byte bFlags;              //01 byte
		public byte bYear, bMonth, bDay; //03 bytes
		public uint dwRecordsCount;     //04 bytes
		public ushort wHeaderSize;         //02 bytes
		public ushort wRecordSize;         //02 bytes
		public ushort wReserved;           //02 bytes
		public byte bTransactionFlags;   //01 byte
		public byte bEncryption;         //01 byte
		public byte[] baUseUserEnvironment; //12 bytes
		public byte bUseIndex;           //01 byte
		public byte bCodePage;           //01 byte
		public ushort wReserved2;          //02 bytes
		public DBFFieldDescriptor[] Field;
		public DBFTable Parent;
// ReSharper restore InconsistentNaming

		//Конструктор
		public DBFHeader( DBFTable parent )
		{
			Parent = parent;
			bFlags = 0;
			bYear = 0;
			bMonth = 0;
			bDay = 0;
			dwRecordsCount = 0;
			wHeaderSize = 0;
			wRecordSize = 0;
			wReserved = 0;
			bTransactionFlags = 0;
			bEncryption = 0;
			baUseUserEnvironment = new byte[12];
			bUseIndex = 0;
			bCodePage = 0;
			wReserved2 = 0;
			Field = new DBFFieldDescriptor[0];// (this);
		}
		public delegate void AfrerConstruction( );
	};

	//Структура, реализующая поле записи в dBase-таблице
// ReSharper disable InconsistentNaming
    public struct DBFField
// ReSharper restore InconsistentNaming
    {
        private readonly byte[] _value;
        private DBFRecord _parent;
        private DBFFieldDescriptor _descriptor;
        
		//Конструктор
        public DBFField( DBFFieldDescriptor descr, DBFRecord pr )
        {
			_parent = pr;
			_descriptor = descr;
			_value = new byte[_descriptor.bFieldSize + _descriptor.bFractionalSize];
			for( ushort x = 0; x < _value.Length; x++ ) _value[x] = 0;
        }

        public DBFDate ToDate()
        {
            DBFDate date;
            DBFTable up = _parent.Parent;

            if( _descriptor.bFieldType != (byte)'D' ) return new DBFDate(0, 0, 0);

            Encoding encoding = Encoding.GetEncoding(up.Options.dwCodePage);
            string s = encoding.GetString(_value);

            date.Year = Convert.ToUInt16(s.Substring(0, 4));
            date.Month = Convert.ToUInt16(s.Substring(4, 2));
            date.Day = Convert.ToUInt16(s.Substring(6, 2));
            return date;
        }
        public double ToDouble()
        {
            DBFTable up = _parent.Parent;

            if( (_descriptor.bFieldType != (byte)'N') && (_descriptor.bFieldType != (byte)'F') )
                return double.NaN;

            Encoding encoding = Encoding.GetEncoding(up.Options.dwCodePage);
            string s = encoding.GetString(_value).Trim();
            return Convert.ToDouble(s);
        }
        public override string ToString()
        {
            DBFTable up = _parent.Parent;

            Encoding encoding = Encoding.GetEncoding(up.Options.dwCodePage);
            switch( _descriptor.bFieldType )
            {
                case (byte)'C': 
                    return encoding.GetString(_value).Trim();

                case (byte)'D': 
                    string ds = encoding.GetString(_value).Trim();
                    return ds.Substring(0, 4) + "/" + ds.Substring(4, 2) + "/" + ds.Substring(6, 2);

                case (byte)'N': 
                    return encoding.GetString(_value).Trim();
                    
                case (byte)'F': 
                    return encoding.GetString(_value).Trim();
                    
                case (byte)'L': 
                    byte c = _value[0];
                    if( (c == (byte)'t') || (c == (byte)'T') || (c == (byte)'y') || (c == (byte)'Y') )
                        return "true";
                    if( (c == (byte)'f') || (c == (byte)'F') || (c == (byte)'n') || (c == (byte)'N') )
                        return "false";
                    return "undefined";
            }
            return string.Empty;
        }

        public bool Set( String strValue )
        {
            DBFTable up = _parent.Parent;
            Encoding encoding = Encoding.GetEncoding(up.Options.dwCodePage);
            var unicodeEncoding = new UnicodeEncoding();

            if( _descriptor.bFieldType != (byte)'C' ) 
                return false;
            
            string s = strValue.Substring(0, _descriptor.bFieldSize);
            byte[] charArray = unicodeEncoding.GetBytes(s);
			Value = Encoding.Convert(unicodeEncoding, encoding, charArray);
            return true;
        }
        public bool Set( DBFDate dateValue )
        {
            //acDBFTable up = _parent.Parent;
            if( _descriptor.bFieldType != (byte)'D' ) return false;

            var unicodeEncoding = new UnicodeEncoding();

            string s = dateValue.ToString();
            Value = unicodeEncoding.GetBytes(s);
            return true;
        }
        public bool Set( double flValue )
        {
            if( (_descriptor.bFieldType != (byte)'F') && (_descriptor.bFieldType != 'N') ) return false;

            var unicodeEncoding = new UnicodeEncoding();

            string s = flValue.ToString();
            string decimator = NumberFormatInfo.CurrentInfo.NumberDecimalSeparator;
            
			var dpos = (byte)s.IndexOf(decimator);
			if( dpos > _descriptor.bFieldSize ) return false;
            
			var epos = (byte)(s.Length - dpos);
			if( epos > _descriptor.bFractionalSize ) epos = _descriptor.bFractionalSize;
            
			string rs = s.Substring(0, dpos) + "." + s.Substring(dpos + 1, epos);
            dpos = (byte)(_descriptor.bFieldSize + _descriptor.bFractionalSize - rs.Length);
            rs = (new string(' ', dpos)) + rs;
            Value = unicodeEncoding.GetBytes(rs);

            return true;
        }
		public DBFRecord Parent { get { return _parent; } }
        public byte[] Value
        {
            get { return _value; }
            set
            {
                int n;
                if( value.Length <= (_descriptor.bFieldSize + _descriptor.bFractionalSize) ) 
                    n = value.Length;
                else 
                    n = (_descriptor.bFieldSize + _descriptor.bFractionalSize);
                for( int i = 0; i < n; i++ ) _value[i] = value[i];
            }
        }
    }

    //Структура, реализующая запись в dBase-таблице
// ReSharper disable InconsistentNaming
    public struct DBFRecord
// ReSharper restore InconsistentNaming
    {
        private readonly DBFTable _parent;
        private byte _bDeletionFlag;
        public DBFField[] Field;

        public DBFTable Parent { get { return _parent; } }
        public byte DelFlag { get{ return _bDeletionFlag; } }

        public DBFRecord( DBFTable prnt, ushort fieldsCount )
        {
            _parent = prnt;
            Field = new DBFField[fieldsCount];
            _bDeletionFlag = 0x22; //" "
        }
        
        public bool Deleted
        {
			get { return _bDeletionFlag != 0x20; }
        }
        public void Delete()
        {
            _bDeletionFlag = 0x2a;  //"*"
        }
        public void Restore()
        {
            _bDeletionFlag = 0x20; //" "
        }
    };

	//Структура, реализующая индекс dBase-таблицы
// ReSharper disable InconsistentNaming
    public struct DBFIndexItem
// ReSharper restore InconsistentNaming
    {
        public uint RecNum;
        public String Key;

		public delegate String IndexItemHandler( String value );
		public static IndexItemHandler ItemHandler;
    };

// ReSharper disable InconsistentNaming
	public struct DBFIndex
// ReSharper restore InconsistentNaming
	{
		private DBFIndexItem[] _items;
		private readonly DBFTable _parent;
		private ushort[] _indexFields;

		//Конструктор
		public DBFIndex( DBFTable parentTable )
		{
			_items = null;
			_parent = parentTable;
			_indexFields = null;
		}

		public bool Create( ushort[] fields )
		{
		    uint i;

		    if( fields == null ) return false;
		    DBFTable parent = _parent;
		    if (fields.Any(m => m > parent.Header.Field.Length))
			    return false;
			
            Clear();

			_indexFields = fields;
			_items     = new DBFIndexItem[_parent.Record.Length];
			var tempIndex = new DBFIndexItem[_parent.Record.Length];

			for( i = 0; i < _parent.Record.Length; i++ )
			{
				_items[i].RecNum = i;

				string sKey = string.Empty;
// ReSharper disable LoopCanBeConvertedToQuery
				foreach( int m in fields ) sKey = sKey + _parent.Record[i].Field[m];
// ReSharper restore LoopCanBeConvertedToQuery
// ReSharper disable ConvertIfStatementToConditionalTernaryExpression
				if( DBFIndexItem.ItemHandler == null ) _items[i].Key = sKey;
// ReSharper restore ConvertIfStatementToConditionalTernaryExpression
					else _items[i].Key = DBFIndexItem.ItemHandler(sKey);//"";
			}

			//первичная сортировка
			for( i = 0; i < _items.Length - 1; i += 2 )
				if( _items[i].Key == _items[i + 1].Key )
				{
				    tempIndex[i] = _items[i]; 
                    tempIndex[i + 1] = _items[i + 1];
				}
                else
				{
				    tempIndex[i] = _items[i + 1];
                    tempIndex[i + 1] = _items[i];
				}
			if( i < _items.Length ) tempIndex[i] = _items[i];

			//инициализация
			DBFIndexItem[] pa = tempIndex;
			DBFIndexItem[] pb = _items;
			uint blockSize = 2;

			//основная сортировка
			while( blockSize < _items.Length ) //perform while block size are less then array size
			{
				i = 0;  //setting base itemses for left block
				uint k = 0;
				while( i < _items.Length )
				{
				    uint j;
				    if( (i + blockSize) >= _items.Length )
					{
						for( j = 0; j < (_items.Length - i); j++ ) pb[k + j] = pa[i + j];
						k += (uint)(_items.Length - i);
						break;
					}

					uint lSize = blockSize;
				    uint rSize;
				    if( (i + blockSize + blockSize) < _items.Length ) rSize = blockSize;
					else rSize = (uint)(_items.Length - i - blockSize);  //set length of right block
					j = i + lSize;   //set base items for right block
					while( (lSize > 0) && (rSize > 0) )  //starting merging of blocks
					{
						if( pa[i].Key.CompareTo(pa[j].Key) <= 0 )
						{
							pb[k] = pa[i];
							i++;
							lSize--;
						}
						else
						{
							pb[k] = pa[j];
							j++;
							rSize--;
						}
						k++;
					}
					if( rSize == 0 )
					{
						for( uint t = 0; t < lSize; t++ ) pb[k + t] = pa[i + t];
						i += lSize;
						k += lSize;
					}
					if( lSize == 0 )
					{
						for( uint t = 0; t < rSize; t++ ) pb[k + t] = pa[j + t];
						j += rSize;
						k += rSize;
					}
					i += blockSize;
				}
				blockSize *= 2; //double size of block
				var cb = pb; 
                pb = pa; 
                pa = cb; //switch input and output arrays
			}
			//if( _items != pa ) 
                _items = pa;

			return true;
		}

		//Очистка индекса
		public void Clear( )
		{
			_items = null;
			_indexFields = null;
		}

		public DBFIndexItem this[ushort idx] { get { return _items[idx]; } }
		public DBFIndexItem this[uint idx] { get { return _items[idx]; } }

		//public int Length { get { return items.Length; } }
		public uint Length { get { return (uint)_items.Length; } }
	}

	//Структура, хранящая управляющие флаги обработки таблицы
// ReSharper disable InconsistentNaming
    public struct DBFOptions
    {
        public bool bKeepDeletedRecords;
        public bool bOverwriteExistsFileOnSave;
        public bool bTrimFieldsOnIndexing;
        public int dwCodePage;
    };
// ReSharper restore InconsistentNaming


	//Класс, реализующий dBase-таблицу
// ReSharper disable InconsistentNaming
    public class DBFTable
// ReSharper restore InconsistentNaming
    {
		//Коды ошибок
        public const byte ErrNoErrors = 0,
                          ErrCantOpenTable = 1,
                          ErrCantSaveTable = 2,
                          ErrWrongFileFormat = 3,
                          ErrWrongValue = 4,
                          ErrWrongIndex = 5,
                          ErrWrongFieldFormat = 6;
                     
        private byte _bLastError;      //Поле, хранящее результат последней операции
        
		//Заголовок таблицы
        public DBFHeader Header;

		//Массив записей таблицы
        public DBFRecord[] Record;

		//Управляющие флаги
        public DBFOptions Options;

		//Массив индексов
        public DBFIndex Index;

		//Конструктор
        public DBFTable(string tablePath)
        {
            Options.bKeepDeletedRecords = false;
            Options.bOverwriteExistsFileOnSave = true;
            
            if(!LoadFrom(tablePath)) return;
        }

		//Загрузка таблицы из файла
        public bool LoadFrom(string tablePath)
        {
            ushort i;
            uint j;

            if( !File.Exists(tablePath) )
            {
                _bLastError = ErrCantOpenTable;
                return false;
            }

			Header = new DBFHeader(this);
			Index = new DBFIndex(this);
            
            var f = new FileStream(tablePath, FileMode.Open, FileAccess.Read);
            var r = new BinaryReader(f);
            Header.bFlags = r.ReadByte();
            Header.bYear = r.ReadByte();
            Header.bMonth = r.ReadByte();
            Header.bDay = r.ReadByte();
            Header.dwRecordsCount = r.ReadUInt32();
            Header.wHeaderSize = r.ReadUInt16();
            Header.wRecordSize = r.ReadUInt16();
            Header.wReserved = r.ReadUInt16();
            Header.bTransactionFlags = r.ReadByte();
            Header.bEncryption = r.ReadByte();
            Header.baUseUserEnvironment = r.ReadBytes(12);
            Header.bUseIndex = r.ReadByte();
            Header.bCodePage = r.ReadByte();
            Header.wReserved2 = r.ReadUInt16();

            var wFieldsCount = (ushort)(((short)Header.wHeaderSize - 32 - 1)/32);
            Header.Field = new DBFFieldDescriptor[wFieldsCount];

            for (i = 0; i < wFieldsCount; i++)
            {
                Header.Field[i] = new DBFFieldDescriptor(Header, i)
                                      {
                                          strName = r.ReadBytes(11),
                                          bFieldType = r.ReadByte(),
                                          dwAddress = r.ReadUInt32(),
                                          bFieldSize = r.ReadByte(),
                                          bFractionalSize = r.ReadByte(),
                                          wReserved = r.ReadUInt16(),
                                          bWorkAreaID = r.ReadByte(),
                                          wMutiUser = r.ReadUInt16(),
                                          bSetFields = r.ReadByte(),
                                          baReserved = r.ReadBytes(7),
                                          bMDXIDIncluded = r.ReadByte()
                                      };
            }
            byte c = r.ReadByte();
            if (c != 13)
            {
                r.Close();
				f.Close();
				_bLastError = ErrWrongFileFormat;
				return false;
            }
            Record = new DBFRecord[Header.dwRecordsCount];
            for (j = 0; j < Header.dwRecordsCount; j++)
            {
                Record[j] = new DBFRecord(this, wFieldsCount);
				if( r.ReadByte() != 0x20 ) Record[j].Delete();
							else Record[j].Restore();
                Record[j].Field = new DBFField[wFieldsCount];

                for (i = 0; i < wFieldsCount; i++)
                {
                    Record[j].Field[i] = new DBFField(Header.Field[i], Record[j])
                                             {Value = r.ReadBytes(Header.Field[i].bFieldSize)};
                }
            }
            r.Close();
            f.Close();
			//r = null;
			//f = null;

			switch( Header.bCodePage )
			{
				case 0x01: Options.dwCodePage = 437; break;
				case 0x02: Options.dwCodePage = 850; break;
				case 0x65: Options.dwCodePage = 866; break;
				case 0x26: Options.dwCodePage = 866; break;
				case 0xC8: Options.dwCodePage = 1250; break;
				default: Options.dwCodePage = 1251; break;
			}

            _bLastError = ErrNoErrors;
            return true;
        }

		//Сохранение таблицы в файл
        public bool SaveTo(String tablePath)
        {
            ushort i;
            uint j;
            byte c = 13;

            if( !this.Options.bOverwriteExistsFileOnSave )
                if( File.Exists(tablePath) )
                {
                    _bLastError = ErrCantSaveTable;
                    return false;
                }

            var f = new FileStream(tablePath, FileMode.Create);
            var w = new BinaryWriter(f);

            if (Options.bKeepDeletedRecords == false) Pack();

            w.Write(Header.bFlags);
            w.Write(Header.bYear);
            w.Write(Header.bMonth);
            w.Write(Header.bDay);
            w.Write(Header.dwRecordsCount); 
            w.Write(Header.wHeaderSize);
            w.Write(Header.wRecordSize);
            w.Write(Header.wReserved);
            w.Write(Header.bTransactionFlags);
            w.Write(Header.bEncryption);
            w.Write(Header.baUseUserEnvironment);
            w.Write(Header.bUseIndex);
            w.Write(Header.bCodePage);
            w.Write(Header.wReserved2);
            for (i = 0; i < Header.Field.Length; i++)
            {
                w.Write(Header.Field[i].strName);
                w.Write(Header.Field[i].bFieldType);
                w.Write(Header.Field[i].dwAddress);
                w.Write(Header.Field[i].bFieldSize);
                w.Write(Header.Field[i].bFractionalSize);
                w.Write(Header.Field[i].wReserved);
                w.Write(Header.Field[i].bWorkAreaID);
                w.Write(Header.Field[i].wMutiUser);
                w.Write(Header.Field[i].bSetFields);
                w.Write(Header.Field[i].baReserved);
                w.Write(Header.Field[i].bMDXIDIncluded);
            }
            w.Write(c);

            for( j = 0; j < Header.dwRecordsCount; j++ )
            {
                w.Write(Record[j].DelFlag);
                for (i = 0; i < Header.Field.Length; i++)
                    w.Write(Record[j].Field[i].Value, 0, Record[j].Field[i].Value.Length);
            }
			c = 26;
			w.Write(c);

            w.Close();
            f.Close();

            _bLastError = ErrNoErrors;
            return true;
        }

		//Упаковка таблицы
        public bool Pack()
        {
            uint i = 0;
            var j = (uint)(Record.Length - 1);
			
			while( i <= j )
			{
				if( !Record[i].Deleted ) i++;
				else
				{
					Record[i] = Record[j];
					j--;
				}
			}

			Header.dwRecordsCount = j + 1;
            return true;
        }

		public DBFTable Rebuild( String[] fields )
		{
			ushort i, j;
			uint k;
		    String name;

		    if( fields == null ) return this;

			//Проверка на идентичность запрошенной и существующей структуры таблицы
			#region Проверка на идентичность запрошенной и существующей структуры таблицы
			Encoding encoding = Encoding.GetEncoding(Options.dwCodePage);
			bool bAllEqual = true;
			for( i = 0; i < fields.Length;  i++ )
			{
				name = encoding.GetString(Header.Field[i].strName).ToUpper();
				name = name.Substring(0, name.IndexOf('\0'));
				fields[i] = fields[i].Trim().ToUpper();
				if( name.CompareTo(fields[i]) != 0 )
				{
					bAllEqual = false;
					break;
				}
			}
			if( bAllEqual ) return this;
			#endregion

			//Создание индекса столбцов
			#region Создание индекса столбцов
			var newColumns = new ushort[fields.Length];
			for( i = 0; i < newColumns.Length; i++ )
				for( j = 0; j < Header.Field.Length; j++ )
				{
					name = encoding.GetString(Header.Field[j].strName).ToUpper();
					if( name.CompareTo(fields[i]) == 0 )
					{
						newColumns[i] = j;
						break;
					}
				}
			#endregion

			//Генерация новой таблицы
			var dbf = new DBFTable("");

			//Генерация нового заголовка таблицы
			#region Генерация нового заголовка таблицы
			dbf.Header = new DBFHeader(dbf)
			                 {
			                     bFlags = Header.bFlags,
			                     bYear = Header.bYear,
			                     bMonth = Header.bMonth,
			                     bDay = Header.bDay,
			                     dwRecordsCount = Header.dwRecordsCount,
			                     wHeaderSize = (ushort) (33 + 32*fields.Length)
			                 };

		    ushort newRecSize = 1;
			for( i = 0; i < fields.Length; i++ )
			{
				newRecSize += Header.Field[newColumns[i]].bFieldSize;
				newRecSize += Header.Field[newColumns[i]].bFractionalSize;
			}
			dbf.Header.wRecordSize = newRecSize; //размер записи

			dbf.Header.wReserved = Header.wReserved; //зарезервировано
			dbf.Header.bTransactionFlags = Header.bTransactionFlags; //флаги передачи
			dbf.Header.bEncryption = Header.bEncryption; //шифрование
			dbf.Header.baUseUserEnvironment = Header.baUseUserEnvironment; //пользовательское окружение
			dbf.Header.bUseIndex = Header.bUseIndex; //использование индекса
			dbf.Header.bCodePage = Header.bCodePage; //кодировка
			dbf.Header.wReserved2 = Header.wReserved2; //зарезервировано

			dbf.Header.Field = new DBFFieldDescriptor[fields.Length];
			for( i = 0; i < dbf.Header.Field.Length; i++ )
			{
				dbf.Header.Field[i] = new DBFFieldDescriptor(dbf.Header, i)
				                          {
				                              strName = Header.Field[newColumns[i]].strName,
				                              bFieldType = Header.Field[newColumns[i]].bFieldType,
				                              dwAddress = Header.Field[newColumns[i]].dwAddress,
				                              bFieldSize = Header.Field[newColumns[i]].bFieldSize,
				                              bFractionalSize = Header.Field[newColumns[i]].bFractionalSize,
				                              wReserved = Header.Field[newColumns[i]].wReserved,
				                              bWorkAreaID = Header.Field[newColumns[i]].bWorkAreaID,
				                              wMutiUser = Header.Field[newColumns[i]].wMutiUser,
				                              bSetFields = Header.Field[newColumns[i]].bSetFields,
				                              baReserved = Header.Field[newColumns[i]].baReserved,
				                              bMDXIDIncluded = Header.Field[newColumns[i]].bMDXIDIncluded
				                          };
			}
			#endregion

			//Заполнение записей новой таблицы
			#region Заполнение записей новой таблицы
			dbf.Record = new DBFRecord[dbf.Header.dwRecordsCount];
            for (k = 0; k < dbf.Header.dwRecordsCount; k++)
            {
                dbf.Record[k] = new DBFRecord(dbf, (ushort)dbf.Header.Field.Length);
				if( Record[k].Deleted ) dbf.Record[k].Delete();
					else dbf.Record[k].Restore();
                dbf.Record[k].Field = new DBFField[dbf.Header.Field.Length];

                for (i = 0; i < dbf.Header.Field.Length; i++)
                {
                    dbf.Record[k].Field[i] = new DBFField(dbf.Header.Field[i], dbf.Record[k])
                                                 {Value = Record[k].Field[newColumns[i]].Value};
                }
            }
			#endregion
			//Создание нового индекса таблицы
			dbf.Index = new DBFIndex(this);

			//Задание управляющих флагов
			dbf.Options = Options;

			return dbf;
		}

		//Статический метод, сливающий несколько таблиц в одну и возвращающий на нее указатель
		//Рекомендуется поле его вызова сделать сборку мусора
		public static DBFTable Merge( DBFTable[] tables )
		{
		    int j, k;
		    //j - номер текущей таблице
						    //k - номер текущей записи в текущей таблице
							

			//Проверка на наличие таблиц для слияния
			if( tables.Length == 0 ) return null;

			//проверка на совпадение форматов таблиц
			//... можно вставить потом
			//... НУЖНО вставить

			int recCountSummary = tables.Sum(dbfTable => dbfTable.Record.Length);

		    //Создадим новую таблицу
			var dbf = new DBFTable("")
			              {   //и зададим её заголовок
			                  Header = tables[0].Header
			              };
			
		    dbf.Header.dwRecordsCount = (uint)recCountSummary;
			dbf.Header.Parent = dbf;
			//и прочие параметры
			dbf.Options = tables[0].Options;
			dbf.Record = new DBFRecord[recCountSummary];
			
			//Зададим индекс
			dbf.Index = new DBFIndex(dbf);

			//Заполняем записи результирующей таблицы
            int i = 0;  //i - номер записи в новой таблице
			for( j = 0; j < tables.Length; j++ )
				for( k = 0; k < tables[j].Record.Length; k++ )
				{
					dbf.Record[i] = new DBFRecord(dbf, (ushort)dbf.Header.Field.Length);
					
					if( tables[j].Record[k].Deleted ) dbf.Record[i].Delete();
						else dbf.Record[i].Restore();
					dbf.Record[i].Field = new DBFField[dbf.Header.Field.Length];

                    int l; //l - номер текущего поля в текущей записи текущей таблицы
				    for( l = 0; l < dbf.Header.Field.Length; l++ )
					{
						dbf.Record[i].Field[l] = new DBFField(dbf.Header.Field[l], dbf.Record[i])
						                             {Value = tables[j].Record[k].Field[l].Value};
					}
					i++;
				}
			return dbf;
		}
		
		//Свойство, возвращающее результат последней операции
		public byte LastError { get { return _bLastError; } }
    }
}