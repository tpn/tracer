pragma synchronous=off;
pragma count_changes=off;
pragma journal_mode=memory;
pragma temp_store=memory;
pragma mmap_size=268435456;

create table if not exists python_function (
    id integer,
    hash integer,
    path_entry_type integer,
    path_atom integer,
    fullname_atom integer,
    modulename_atom integer,
    name_atom integer,
    classname_atom integer,
    path text,
    fullname text,
    modulename text,
    name text,
    classname text,
    code_object_id integer,
    code_object_hash integer,
    first_line_number integer,
    last_line_number integer,
    number_of_lines integer
);

create table if not exists python_trace_event (
    id integer,
    sequence_id integer,
    trace_event_type integer,
    elapsed integer,
    python_function_id integer,
    python_function_hash integer,
    code_object_hash integer,
    linenumber integer
);
