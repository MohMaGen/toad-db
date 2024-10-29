*****************************
TQL - toad-db query language.
*****************************

DOMENS:
=======

numeric domens:
---------------
* unsiged integers: **U8, U16, U32, U64, U128**.
* signed integers: **I8, I16, I32, I64, I128**.
* real numbers: **F32, F64, F128**
* boolean domen: **Bool** can be one of true, false.


Time and Date:
--------------
* Month can be one of **jun | feb | mar | apr | may | jul | jun | aug | sep | oct | nov | dec**.
* Day alias to U8.
* Year alias to U16.
* Seconds alias to U32.
* Date structured as **day(Day) &  month(Month)  &  year(Year) & time(Seconds)**.
* Time_Stamp alias to U64.

Special domens:
---------------
* Key alias to U64. Used for keys.

Define custom domen:
--------------------
.. code-block::rust
    domen ID := U8; // just an alias
    domen ERR := wrong_addr | wrong_usr_name; // just variants
    domen Vector := v2(F64 & F64) | v3(F64 & F64 & F64) | v4(F64 & F64 & F64 & F64); // nested declaration

Different ways to create domen.

TABLES:
=======

Create global table:
-------------------
.. code-block::rust
    table Groups {
        key(Key): unique? not_null? Num::inc!,
        title(U8[32]): unique? Arr::not_null?,
        level(U8): default(0),
    };

    table Users {
        key(Key): unique? Num::not_null? Num::inc(User.key)!,
        name(U8[256]): Arr::not_null?,
        group_key(Key): Key::from_table(Groups, key)?,
    };

* ``:`` -- means rules on create.
* ``?`` -- means predicate rule.
* ``!`` -- means generate rule.

Table operations
----------------
* ``@``, ``Table`` => get global table by its name.
* ``<,>``, ``Table_Name<column_name1, column_name2>`` => create new tmp table with the same rows but only with provided columns.
* ``(,)``, ``Table_Name{new_name1, new_name2}`` => create new tmp table with new columns names.
* ``*``, ``Table1 * Table2`` => create new table with all possible combinations of two tables.
* ``?=>``, ``Table ?=> cond`` => filter rows of Table.
* ``[:]``, ``[row_id:columns,]`` => return list of elements or one element with row by idx in table.


Query data:
-----------
.. code-block::rust
    display @Groups; // print all collumns and rows of table.

    display @Groups<title>; // print titles of all rows.

    display @Groups<title>{t}; // print titles as t of all rows.

    display @Users * @Groups; // print all columns and rows of table consit of all possible combinations of two tables.

    display (@Users<name, group_key>{name, uk} * @Groups<title, key>{title, gk}
                ?=> gk == uk)<name, title>; // print all users names with their group titles. Display do not show columns with names starts with _.

Command ``display`` prints table. You can create table using Table manipulations.

Insert data:
------------
.. code-block::rust
    Groups << { title: "admin", level: 2 } << { title: "mod", level: 1 } << { title: "user" };
    Users  << { name: "Aboba", group_key: 0 } << { title: "Vasya", group_key: (@Groups<key,title> ?=> title == "user")[0:key} };
