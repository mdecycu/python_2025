# 從 pybean.py 導入 Store 與 SQLiteWrite 模組
from pybean import Store, SQLiteWriter
# 預計將資料存入 book.db, 且將 frozen 設為 False 表示隨時可以修改資料庫中資料表的欄位
library = Store(SQLiteWriter("book.db", frozen=False))
# 在 library 物件中建立一個資料表, 名稱為 "book", 可以將 book 變數設想為資料庫物件
book = library.new("book")
# 資料表中的 title 欄位內容設定
book.title = "Boost development with pybean"
# 資料表中的 author 欄位內容設定
book.author = "Charles Xavier"
# 將上述的兩個資料表欄位內容存檔
library.save(book)
# 資料存入資料庫之後, 就可以從資料庫中的資料表 book 查詢特定內容
for book in library.find("book","author like ?",["Charles Xavier"]):
        print(book.title)
# 準備利用 delete 方法刪除 book 資料表
#library.delete(book)
# 提交上述的資料庫操作
library.commit()