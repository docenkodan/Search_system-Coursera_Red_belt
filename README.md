# Search system - Coursera Red belt (Final task)
### Дата создания: 17.09.2020
Реализация многопоточной поисковой системы. 

Класс **SearchServer** позволяет выполнять поиск в базе документов. 

    class SearchServer {
    public:
      SearchServer() = default;
      explicit SearchServer(istream& document_input);

      void UpdateDocumentBase(istream& document_input);
      void AddQueriesStream(istream& query_input, ostream& search_results_output);
    };


### Конструктор
Конструктор класса принимает поток ввода, содержащий базу документов.
* один документ — это одна строка входного потока;
* документы состоят из слов, разделённых одним или несколькими пробелами;
* слова состоят из строчных латинских букв. Например, код, приведённый ниже, загружает в объект класса SearchServer базу из трёх документов:

      string docs;
      docs += "london is the capital of great britain\n";
          // документ содержит 7 слов
      docs += "i am travelling down the river\n";
          // документ содержит 6 слов
      docs += "  here     we    go             ";
          // документ содержит 3 слова

      istringstream document_input(docs);
      SearchServer srv(document_input);

### Метод AddQueriesStream
Метод **AddQueriesStream** выполняет поиск в документах и поддерживает асинхронный вызов.

Метод принимает входной поток поисковых запросов и выходной поток для записи результатов поиска.
* один запрос — это одна строка в потоке query_input;
* каждый поисковый запрос состоит из слов, разделённых одним или несколькими пробелами;
* так же, как и в документах, слова в запросах состоят из строчных латинских букв.

Результатом обработки поискового запроса является набор из максимум пяти наиболее релевантных документов. В качестве метрики релевантности используется суммарное количество вхождений всех слов запроса в документ. 

***Например***, у нас есть поисковая база из трёх документов: "*london is the capital of great britain*", "*moscow is the capital of the russian federation*", "*paris is the capital of france*", — и поисковый запрос "*the best capital*". Тогда метрика релевантности у документов будет следующей:

* *london is the capital of great britain* — **2** (слово "*the*" входит в документ 1 раз, слово "*best*" — ни разу, слово "*capital*" — 1 раз);
* *moscow is the capital of the russian federation* — **3** (слово "*the*" входит в документ 2 раза, слово "*best*" — ни разу, слово "*capital*" — 1 раз);
* *paris is the capital of france* — **2** ("*the*" — 1, "*best*" — 0, "*capital*" — 1).

В итоге получается, что документ "*moscow is the capital of the russian federation*" оказывается наиболее релевантным запросу "*the best capital*".

Для каждого поискового запроса метод **AddQueriesStream** выводит в поток **search_results_output** одну строку в формате

    [текст запроса]: {docid: <значение>, hitcount: <значение>} {docid: <значение>, hitcount: <значение>} ...
, где **docid** — идентификатор документа, а **hitcount** — значение метрики релевантности для данного документа (то есть суммарное количество вхождений всех слов запроса в данный документ).

Результаты поиска не включают документы, **hitcount** которых равен нулю.
При подсчёте **hitcount** учитываются только слова целиком, то есть слово «*there*» не является вхождением слова «*the*».

### Метод UpdateDocumentBase
Метод **UpdateDocumentBase** заменяет текущую базу документов на новую, которая содержится в потоке **document_input**. При этом документ из первой строки этого потока будет иметь идентификатор (*docid*) **0**, документ из второй строки — идентификатор **1** и т.д. Точно так же идентификаторы документам назначает и **конструктор** класса **SearchServer**.

Метод поддерживает асинхронный вызов. Для того, чтобы процесс поиска не останавливался, в отдельном потоке создает объект класса InvertedIndex. Процесс поиска незначительно приостанавливается во время перемещения созданного объекта в поле index класса SearchServer.

Например, код:

    const string doc1 = "london is the capital of great britain";
    const string doc2 = "moscow is the capital of the russian federation";
    istringstream doc_input1(doc1 + '\n' + doc2);
    SearchServer srv(doc_input1);

    const string query = "the capital";
    istringstream query_input1(query);
    srv.AddQueriesStream(query_input1, cout);

    istringstream doc_input2(doc2 + '\n' + doc1);
    srv.UpdateDocumentBase(doc_input2);
    istringstream query_input2(query);
    srv.AddQueriesStream(query_input2, cout);
выводит:

    the capital: {docid: 1, hitcount: 3} {docid: 0, hitcount: 2}
    the capital: {docid: 0, hitcount: 3} {docid: 1, hitcount: 2}

### Класс InvertedIndex
Для каждого слова строит список документов, в котором оно встречается. Таким образом достигается лучшая асимптотики обработки поискового запроса.

[Об Инвертированном индексе на Wikipedia](https://ru.wikipedia.org/wiki/%D0%98%D0%BD%D0%B2%D0%B5%D1%80%D1%82%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%BD%D1%8B%D0%B9_%D0%B8%D0%BD%D0%B4%D0%B5%D0%BA%D1%81).



