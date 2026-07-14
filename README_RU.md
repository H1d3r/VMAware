<div align="center">
   <img src="assets/banner.jpg" width="500" title="VMAware">
   <br>
   <img src="https://img.shields.io/github/actions/workflow/status/NotRequiem/VMAware/build_infrastructure.yml">
   <img src="https://img.shields.io/github/downloads/NotRequiem/VMAware/total">
   <img src="https://img.shields.io/badge/License-MIT-yellow.svg">
   <a href="https://deepwiki.com/NotRequiem/VMAware"><img src="https://deepwiki.com/badge.svg" alt="Ask DeepWiki"></a>
   <br><br>
   <b>VMAware</b> (VM + Aware) — это кроссплатформенный C++ фреймворк для обнаружения виртуальных машин.
   <br><br>
   <a href="README.md">English 🇬🇧</a> | <a href="README_CN.md">中文 🇨🇳</a> | <a href="README_FR.md">Français 🇫🇷</a> | <a href="README_KR.md">한국어 🇰🇷</a>
</div>

- - -

Библиотека:

- Очень проста в использовании
- Кроссплатформенна (Windows + MacOS + Linux)
- Поддерживает множество архитектур (amd64, arm64, armhf, armel, i386, mips64el, ppc64el, riscv64, s390x)
- Включает около 90 уникальных техник обнаружения ВМ [[список](https://github.com/NotRequiem/VMAware/blob/main/docs/documentation.md#flag-table)]
- Использует самые передовые технологии
- Способна обнаруживать около 70 марок ВМ, включая VMware, VirtualBox, QEMU, Hyper-V и многие другие [[список](https://github.com/NotRequiem/VMAware/blob/main/docs/documentation.md#brand-table)]
- Преодолевает защиту (харденинг) ВМ
- Очень гибкая, с полным контролем над выполняемыми техниками
- Обнаруживает различные VM- и полу-VM-технологии: гипервизоры, эмуляторы, контейнеры, песочницы и т.д.
- Доступна для C++11 и новее
- Содержит экосистему портов на другие языки: Rust, JavaScript и Ruby
- Реализована в виде одного заголовочного файла (header-only)
- Не требует внешних зависимостей
- Использует мемоизацию (кэширование результатов для повышения производительности)
- Полностью лицензирована под MIT, что допускает неограниченное использование и распространение

<br>

> [!NOTE]
> Мы ищем переводчиков, готовых перевести этот README на их родной язык. Если хотите внести вклад, смело отправляйте PR!


## Пример 🧪

```cpp
#include "vmaware.hpp"
#include <iostream>

int main() {
    if (VM::detect()) {
        std::cout << "Virtual machine detected!" << "\n";
    } else {
        std::cout << "Running on baremetal" << "\n";
    }

    std::cout << "VM name: " << VM::brand() << "\n";
    std::cout << "VM type: " << VM::type() << "\n";
    std::cout << "VM certainty: " << (int)VM::percentage() << "%" << "\n";
    std::cout << "VM hardening: " << (VM::is_hardened() ? "likely" : "not found") << "\n";
}
```

Возможный вывод:

```
Virtual machine detected!
VM name: VirtualBox
VM type: Hypervisor (type 2)
VM certainty: 100%
VM hardening: not found
```

<br>

## Структура ⚙️

<p align="center">
<img src="assets/vmaware.png" align="center" title="VMAware">
<br>
</p>

<br>

## CLI-утилита 🔧
Проект также предоставляет удобную CLI-утилиту, использующую полный потенциал библиотеки. Она также кроссплатформенна.

Ниже приведён пример базовой системы QEMU без модификаций защиты на Linux.

<img src="assets/demo.png" title="cli">

<br>

## Установка 📥

Чтобы установить библиотеку, скачайте файл `vmaware.hpp` из последнего [релиза](https://github.com/NotRequiem/VMAware/releases/latest) в ваш проект. Там же находятся бинарники. Никаких CMake или привязок разделяемых объектов не требуется — всё действительно так просто.

Если же вы хотите полный проект (глобально доступный заголовочный файл `<vmaware.hpp>` и CLI-утилиту), выполните следующие команды:

```bash
git clone https://github.com/NotRequiem/VMAware 
cd VMAware
```

### ДЛЯ LINUX:

```bash
sudo dnf/apt/yum update -y # замените на ваш пакетный менеджер
mkdir build
cd build
cmake ..
sudo make install
```

### ДЛЯ MACOS:

```bash
mkdir build
cd build
cmake ..
sudo make install
```

### ДЛЯ WINDOWS:

```bash
cmake -S . -B build/ -G "Visual Studio 16 2019"
```

Для отладочной сборки добавьте `-DCMAKE_BUILD_TYPE=Debug` к аргументам cmake.

<br>

### Установка через CMake

```cmake
# отредактируйте это
set(DIRECTORY "/path/to/your/directory/")

set(DESTINATION "${DIRECTORY}vmaware.hpp")

if (NOT EXISTS ${DESTINATION})
    message(STATUS "Downloading VMAware")
    set(URL "https://github.com/NotRequiem/VMAware/releases/latest/download/vmaware.hpp")
    file(DOWNLOAD ${URL} ${DESTINATION} SHOW_PROGRESS)
else()
    message(STATUS "VMAware already downloaded, skipping")
endif()
```

Файл модуля и версия-функция находятся [здесь](auxiliary/vmaware_download.cmake)

<br>

## Документация и обзор кода 📒

Полная документация доступна [здесь](docs/documentation.md). Все детали: функции, техники, настройки и примеры. Поверьте, это не так страшно ;)

Об архитектуре и дизайне библиотеки можно узнать на https://deepwiki.com/NotRequiem/VMAware.

Информация в DeepWiki или других неофициальных источниках может быть неточной.

<br>

## Порты на другие языки 🔀

VMAware также поддерживается на других языках. Если C++ — не то, что вам нужно, обратитесь к списку ниже. Все эти проекты официально одобрены разработчиками VMAware.

| Язык | Репозиторий | Подробности | Автор |
|:---------|:---------------:|:--------:|:------:|
| <img src="assets/language_icons/8172-ruby-programming-language.webp" width="20"> Ruby | [ссылка](https://github.com/NotRequiem/VMAware/tree/main/gem) | Официальный порт на Ruby встроен в репозиторий VMAware, но Windows не поддерживается. | [Adam Ruman](https://github.com/addam128) |
| <img src="assets/language_icons/js.png" width="20"> JS | [ссылка](https://github.com/Kyun-J/node-vm-detect) | Отличное API, активно поддерживается. | [Kyun-J](https://github.com/Kyun-J) |
| <img src="assets/language_icons/rust-logo-512x512.png" width="20"> Rust | [ссылка](https://github.com/MarcelDev/vmaware-rs) | Отличное API, хорошо протестирован, активно поддерживается | [Marcel](https://github.com/MarcelDev) |

> [!WARNING]
> Неофициальные порты существуют, но они не так тщательно протестированы, как официальные. Кроме того, все порты подвержены потенциальным ложным срабатываниям из-за сложности исходного C++ кода. За пределами этого списка порты, созданные с помощью ИИ, неспособны точно воспроизвести наши техники (или полностью непригодны). Используйте их на свой страх и риск.

<br>

## Q&A ❓

<details>
<summary>Как это работает?</summary>
<br>

> Библиотека использует всеобъемлющий список низкоуровневых и высокоуровневых анти-VM техник, учитываемых в системе баллов. Баллы (0–100) для каждой техники назначаются на основе объективных критериев, направленных на обнаружение самых скрытных ВМ с минимальным количеством ложных срабатываний. Каждая техника, обнаружившая ВМ, добавляет свои баллы в общую накопительную сумму, и превышение порогового значения определяет, работает ли система внутри ВМ.

</details>

<details>
<summary>Для кого эта библиотека и какие у неё случаи применения?</summary>
<br>

> Она предназначена для исследователей безопасности, инженеров ВМ, разработчиков античитов и всех, кому нужен практичный и надёжный механизм обнаружения ВМ в своём проекте. Библиотека полезна аналитикам вредоносного ПО, тестирующим скрытность своих ВМ, и разработчикам проприетарного ПО, защищающим свои приложения от реверс-инжиниринга. Это эффективный инструмент для оценки того, насколько хорошо ВМ может скрыться от обнаружения.
> 
> Кроме того, программа может менять своё поведение в зависимости от обнаруженного окружения. Это может быть полезно для отладки и тестирования, а системные администраторы могут гибко управлять конфигурациями. Некоторые приложения могут юридически ограничивать использование в ВМ в лицензионном соглашении для предотвращения несанкционированного распространения или тестирования.
>
> Существуют проекты, использующие наш инструмент, например [AutoVirt](https://github.com/Scrut1ny/AutoVirt) — продвинутый проект анализа вредоносного ПО, которому мы помогли укрепить среду гипервизора и необнаруживаемость.

</details>

<details>
<summary>Почему ещё один проект для обнаружения ВМ?</summary>
<br>

> Уже существует множество проектов с той же целью, например:
<a href="https://github.com/CheckPointSW/InviZzzible">InviZzzible</a>, <a href="https://github.com/a0rtega/pafish">pafish</a> и <a href="https://github.com/LordNoteworthy/al-khaser">Al-Khaser</a>. Но отличие в том, что они не предоставляют программного интерфейса для взаимодействия с механизмами обнаружения, а также практически не поддерживают не-Windows системы. Кроме того, их техники обнаружения ВМ часто недостаточно совершенны для реального применения и не обеспечивают достаточного количества методов обнаружения. Дополнительное препятствие — все они под лицензией GPL, что исключает их использование в проприетарных проектах (основной аудитории для такого функционала).
>
> Pafish и InviZzzible заброшены годами. Хотя Al-Khaser получает occasional обновления и имеет широкий спектр детектов (анти-отладка, анти-инъекции и т.д.), которыми VMAware не обладает, он всё ещё уступает из-за вышеупомянутых проблем.
>
> Хотя эти проекты в какой-то мере были полезны для VMAware, мы хотели сделать их гораздо лучше. Наша цель — сделать техники обнаружения программно доступными, кроссплатформенными и гибкими, чтобы каждый мог извлечь из них пользу, а не просто получить CLI-утилиту. Фреймворк также содержит большее количество техник, так что это, по сути, детектор ВМ на стероидах, ориентированный на практическое и реалистичное применение в любом сценарии.

</details>

<details>
<summary>Не делает ли открытый исходный код проект хуже?</summary>
<br>

> Единственный недостаток VMAware в том, что он полностью открыт, что облегчает задачу обходчикам по сравнению с закрытым исходным кодом. Однако мы считаем это достойным компромиссом ради максимального количества техник обнаружения ВМ в открытом и интерактивном формате, вместо попыток обфускации. Открытый исходный код позволяет получать ценные отзывы сообщества, усиливая библиотеку более эффективно и точно через обсуждения, сотрудничество и соревнование с анти-анти-VM проектами и инструментами анализа вредоносного ПО.
>
> Всё это в совокупности продвинуло инновации в области обнаружения ВМ гораздо продуктивнее, чем если бы код был закрытым. Именно это сделало проект лучшим фреймворком для обнаружения ВМ, и его обход оказался чрезвычайно сложной задачей из-за огромного количества сложных и ранее невиданных техник, которые другие детекторы (как открытые, так и закрытые) не используют (насколько нам известно).
>
> Иными словами, это лучшее качество И количество, лучшая обратная связь и большая открытость вместо безопасности через обфускацию. Та же причина, по которой OpenSSH, OpenSSL, ядро Linux и другие проекты, связанные с безопасностью, относительно защищены: больше людей помогают их улучшать, чем пытаются исследовать исходный код со злым умыслом. VMAware придерживается этой философии. Если вы хоть что-то знаете о безопасности, вам знакомо выражение: «Безопасность через обфускацию — НЕ безопасность».

</details>

<details>
<summary>Насколько эффективны средства защиты (харденинг) ВМ против библиотеки?</summary>
<br>

> Публично известные средства защиты неэффективны, и большинство из них под Windows были побеждены. Но это не значит, что библиотека полностью иммунна. Кастомные средства защиты, о которых мы можем не знать, могут иметь теоретическое преимущество, но их создание значительно сложнее.

</details>

<details>
<summary>Как разрабатывается проект?</summary>
<br>

> На основе онлайн-исследований (от научных статей до приватных форумов по взлому игр и Discord-серверов) мы выявляем методы, используемые для сокрытия ВМ, и разрабатываем общие техники обнаружения, постоянно отслеживая их активность, чтобы оставаться на шаг впереди.
>
> Как только код готов к продакшену, мы загружаем его напрямую в main-ветку и начинаем тестирование в реальных средах.
>
> Продукты, использующие нашу библиотеку, запускают алгоритмы обнаружения на сотнях или даже тысячах устройств и незаметно сообщают, если обнаружена ВМ; эти отчёты вручную проверяются на ложные срабатывания.
>
> Если экспериментальные тесты и публичная документация/базы данных показывают, что ложные срабатывания устранены, мы оставляем изменения в main и присваиваем баллы новым детектам на основе их эффективности, надёжности и взаимодействия с другими техниками.
>
> Исправления для других случаев (ошибки компиляции, потенциальные уязвимости и т.д.) также немедленно вливаются в main.
>
> Когда библиотека накапливает достаточно изменений по сравнению с предыдущими версиями, мы публикуем релиз и подробно описываем изменения.

</details>

<details>
<summary>А как насчёт использования для вредоносного ПО?</summary>
<br>

> Этот проект не способствует разработке вредоносного ПО по очевидным причинам. Даже если вы намерены использовать его для скрытия, антивирусы, скорее всего, пометят его как угрозу, и код не обфусцирован.
>
> Мы намеренно не разрабатываем библиотеку для попыток обхода EDR-флагов, таких как прямой/косвенный вызов системных функций, обнаружение инлайн-хуков и любые другие техники уклонения вредоносного ПО, не связанные с обнаружением виртуализации.

</details>

<details>
<summary>Планируется ли компонент уровня ядра?</summary>
<br>

> Нет. Компонент ядра потребовал бы серьёзного аудита и цифровой подписи драйвера. К тому же это было бы тупиком для обхода ВМ (так что это невесело >:( )
>
> Резюмируя: мы всё ещё можем обнаружить вас, оставаясь полностью в пользовательском режиме.

</details>

<details>
<summary>Потокобезопасна ли библиотека?</summary>
<br>

> Нет. Не вызывайте эту библиотеку из нескольких потоков одновременно — она выполняется меньше 1 секунды.

</details>

<details>
<summary>Как скомпилировать VMAware для старых версий Windows?</summary>
<br>

> По умолчанию VMAware нацелена на Windows 10–11. Если нужно скомпилировать для более старых версий, укажите целевую платформу с помощью макроса WIN32. Например, для Windows 7 добавьте `#define _WIN32_WINNT _WIN32_WINNT_WIN7` в начало `vmaware.hpp`.
>
> В старых терминалах Windows ANSI-цвета не поддерживаются, но можно запустить CLI с аргументом `--no-ansi`. Версии старше Windows 7 НЕ поддерживаются.

</details>

<details>
<summary>У меня ошибки линковки при компиляции</summary>
<br>

> Если вы компилируете с помощью gcc или clang, добавьте флаги `-lm` и `-lstdc++`, или используйте компиляторы g++/clang++. Если ошибки линковки возникают в новом VM-окружении на Linux, обновите систему с помощью `sudo apt/dnf/yum update -y` для установки необходимых компонентов C++.

</details>

<br>

## Вопросы, обсуждения, пул-реквесты и запросы 📬

Если у вас есть предложения, идеи или вы хотите внести вклад — смело спрашивайте! Буду рад обсудить всё в [Issues](https://github.com/NotRequiem/VMAware/issues) или [Discussions](https://github.com/NotRequiem/VMAware/discussions). Если хотите связаться лично — пишите `shenzken` в Discord.

По электронной почте: `vmaware.support@gmail.com`

И если проект оказался полезным, звезда будет очень кстати :)

<br>

## Авторы, участники и благодарности ✒️

<a href="https://github.com/NotRequiem/VMAware/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=NotRequiem/VMAware" />
</a>

<br>

- [Requiem](https://github.com/NotRequiem) (Main developer)
- [kernelwernel](https://github.com/kernelwernel) (Former creator and developer of the project)
- [Check Point Research](https://research.checkpoint.com/)
- [Unprotect Project](https://unprotect.it/)
- [Al-Khaser](https://github.com/LordNoteworthy/al-khaser)
- [pafish](https://github.com/a0rtega/pafish)
- [Matteo Malvica](https://www.matteomalvica.com)
- N. Rin, EP_X0FF
- [Peter Ferrie, Symantec](https://github.com/peterferrie)
- [Graham Sutherland, LRQA Nettitude](https://www.nettitude.com/uk/)
- [Alex](https://github.com/greenozon)
- [Marek Knápek](https://github.com/MarekKnapek)
- [Vladyslav Miachkov](https://github.com/fameowner99)
- [(Offensive Security) Danny Quist](chamuco@gmail.com)
- [(Offensive Security) Val Smith](mvalsmith@metasploit.com)
- Tom Liston + Ed Skoudis
- [Tobias Klein](https://www.trapkit.de/index.html)
- [(S21sec) Alfredo Omella](https://www.s21sec.com/)
- [hfiref0x](https://github.com/hfiref0x)
- [Waleedassar](http://waleedassar.blogspot.com)
- [一半人生](https://github.com/TimelifeCzy)
- [Thomas Roccia (fr0gger)](https://github.com/fr0gger)
- [systemd project](https://github.com/systemd/systemd)
- mrjaxser
- [iMonket](https://github.com/PrimeMonket)
- Eric Parker's discord community
- [ShellCode33](https://github.com/ShellCode33)
- [Georgii Gennadev (D00Movenok)](https://github.com/D00Movenok)
- [utoshu](https://github.com/utoshu)
- [Jyd](https://github.com/jyd519)
- [git-eternal](https://github.com/git-eternal)
- [dmfrpro](https://github.com/dmfrpro)
- [Teselka](https://github.com/Teselka)
- [Kyun-J](https://github.com/Kyun-J)
- [luukjp](https://github.com/luukjp)
- [Randark](https://github.com/Randark-JMT)
- [Scrut1ny](https://github.com/Scrut1ny)
- [Lorenzo Rizzotti (Dreaming-Codes)](https://github.com/Dreaming-Codes)
- [virtfunc](https://github.com/virtfunc)
- [Adam Ruman](https://github.com/addam128)
- [Juan Diego](https://github.com/w451)
- [Wiisus](https://github.com/wiisus)
- [Marcel](https://github.com/MarcelDev)

<br>

## Юридическая информация 📜

Я не несу ответственности за любой ущерб, причинённый злонамеренным использованием этого проекта.

Лицензия: MIT
