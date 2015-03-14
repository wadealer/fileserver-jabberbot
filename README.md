Проект представляет собой джаббер бота "Are There New Files?", позволяющего узнать пользователям вашего файл-сервера, какие новые файлы были залиты за определенный промежуток времени.


Данный бот построен на основе xmpp библиотеки gloox, найти которую можно по этому адресу: http://camaya.net/glooxdownload

Для компиляции бота в папке с исходными кодами просто дайте команды:

$ qmake && make

Либо можете воспользоваться IDE QT Creator. Бот прекрасно компилируется под Windows и Ubuntu Linux. В других ОС я не проверял, но проблем быть не должно. После компиляции бота, полученный бинарник можете разместить в любом удобном вам месте. Главное, чтобы рядом с ним лежал файл настроек settings.ini, а также файл, в котором находится текст помощи - help.txt.

Перед использованием необходимо отредактировать файл settings.ini. Необходимо указать Jid бота(вместе с ресурсом), пароль бота. Опционально указываются настройки прокси-сервера, а также хост и порт сервера. Параметры prefix и separator нужны для формирования прямых ссылок на файлы. Отредактируйте их в соответствии с именем Вашего сервера. В переменных path указываются пути к каталогам, за которыми будет осуществляться наблюдение. В переменной ext можно указать, за файлами с какими расширениями будет вестись наблюдение. Если переменная не задана - наблюдение ведется за всеми файлами. В переменной admin можно задать админский джид, с которого будут доступны команды Send, Quit и Multysend. Если админ не задан - управляющие команды доступны для всех.

Список доступных команд:

*Help - помощь, выводится содержимое файла помощи;

*Since yyyyMMdd - выводит файлы, залитые на сервер начиная с даты. (пример: *Since 20090821);

*All - выводит все файлы, находящиеся на сервере;

*New - новые файлы, появившиеся после последнего запроса New;

*Sub - подписаться на ежедневную рассылку о новых файлах(рассылка приходит в 13-00);

*Unsub - отказаться от рассылок.

Следующие команды предназначены для администраторов сервера, поэтому их описание не стоит выводить в Хэлп ;)

*Send jid@sever: text - отсылает text на указанный jid. Внимание! Эта команда может вызвать ошибки, если не будет соблюден указанный синтаксис;

*Restart - перезагрузка бота;

*Quit - выход из программы;

*Multysend text - отправит text всем зарегистрированным пользователям.

Если вам понравилась эта программа, можете помочь автору ;)

WebMoney:

Z329515322408

R247322956029

U155381744183
