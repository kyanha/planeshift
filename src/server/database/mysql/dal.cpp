/*
 * dbprofile.cpp by Keith Fulton
 *
 * Copyright (C) 2002 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (version 2 of the License)
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <psconfig.h>
#include <csutil/stringarray.h>

#include "util/log.h"
#include "util/consoleout.h"

#define USE_DELAY_QUERY
#include "dal.h"

#define THREADED_BUFFER_SIZE 300

/**
 *
 * Standard SCF stuff to make plugin work
 *
 */

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE(psMysqlConnection)
    SCF_IMPLEMENTS_INTERFACE(iComponent)
    SCF_IMPLEMENTS_INTERFACE(iDataConnection)
SCF_IMPLEMENT_IBASE_END

// more SCF definitions
 
SCF_IMPLEMENT_FACTORY(psMysqlConnection)

/**
 * Actual class which does all the work now.
 */

psMysqlConnection::psMysqlConnection(iBase *iParent)
{
    SCF_CONSTRUCT_IBASE (iParent);
    conn = NULL;
}

psMysqlConnection::~psMysqlConnection()
{
    mysql_close(conn);
    conn = NULL;

    SCF_DESTRUCT_IBASE();
}

bool psMysqlConnection::Initialize (iObjectRegistry *objectreg)
{
    objectReg = objectreg;

    pslog::Initialize (objectreg);

    return true;
}

bool psMysqlConnection::Initialize(const char *host, unsigned int port, const char *database,
                              const char *user, const char *pwd)
{
    // Create a mydb
    mysql_library_init(0, NULL, NULL);

#ifdef USE_DELAY_QUERY
    dqm.AttachNew(new DelayedQueryManager(host,port,user,pwd,database));
    dqmThread.AttachNew(new Thread(dqm));
    dqmThread->Start();
#endif    

    mysql_thread_init();
    conn=mysql_init(NULL);
    // Conn is the valid connection to be used for mydb. Have to store the mydb to get
    // errors if this call fails.
    MYSQL *conn_check = mysql_real_connect(conn,host,user,pwd,database,port,NULL,CLIENT_FOUND_ROWS);
    my_bool my_true = true;
    mysql_options(conn_check, MYSQL_OPT_RECONNECT, &my_true);

    return (conn == conn_check);
}

bool psMysqlConnection::Close()
{
    mysql_close(conn);
    conn = NULL;
    mysql_thread_end();

#ifdef USE_DELAY_QUERY
    dqm->Stop();
    dqmThread->Stop();
#endif    

    mysql_library_end();
    return true;
}


int psMysqlConnection::IsValid()
{
    return (conn) ? 1 : 0;
}

const char *psMysqlConnection::GetLastError()
{
    return mysql_error(conn);
}

// Sets *to to the escaped value
void psMysqlConnection::Escape(csString& to, const char *from)
{
    size_t len = strlen(from);
    char* buff = new char[len*2+1];
  
    mysql_escape_string(buff, from, (unsigned long)len);
    to = buff;
    delete[] buff;
}

unsigned long psMysqlConnection::CommandPump(const char *sql,...)
{
#ifdef USE_DELAY_QUERY
    psStopWatch timer;
    csString querystr;
    va_list args;

    va_start(args, sql);
    querystr.FormatV(sql, args);
    va_end(args);
    dqm->Push(querystr);

    return 1;
#else
    psStopWatch timer;
    csString querystr;
    va_list args;

    va_start(args, sql);
    querystr.FormatV(sql, args);
    va_end(args);

    lastquery = querystr;

    timer.Start();
    if (!mysql_query(conn, querystr))
    {
        if(timer.Stop() > 1000)
        {
            csString status;
            status.Format("SQL query %s, has taken %u time to process.\n", querystr.GetData(), timer.Stop());
            if(LogCSV::GetSingletonPtr())
                LogCSV::GetSingleton().Write(CSV_STATUS, status);
        }
        profs.AddSQLTime(querystr, timer.Stop());
        return (unsigned long) mysql_affected_rows(conn);
    }
    else
        return QUERY_FAILED;
    
#endif    
}

unsigned long psMysqlConnection::Command(const char *sql,...)
{
    psStopWatch timer;
    csString querystr;
    va_list args;

    va_start(args, sql);
    querystr.FormatV(sql, args);
    va_end(args);

    lastquery = querystr;

    timer.Start();
    if (!mysql_query(conn, querystr))
    {
        if(timer.Stop() > 1000)
        {
            csString status;
            status.Format("SQL query %s, has taken %u time to process.\n", querystr.GetData(), timer.Stop());
            if(LogCSV::GetSingletonPtr())
                LogCSV::GetSingleton().Write(CSV_STATUS, status);
        }
        profs.AddSQLTime(querystr, timer.Stop());
        return (unsigned long) mysql_affected_rows(conn);
    }
    else
        return QUERY_FAILED;
}

iResultSet *psMysqlConnection::Select(const char *sql, ...)
{
    psStopWatch timer;
    csString querystr;
    va_list args;

    va_start(args, sql);
    querystr.FormatV(sql, args);
    va_end(args);

    lastquery = querystr;

    timer.Start();
    if (!mysql_query(conn, querystr))
    {
        if(timer.Stop() > 1000)
        {
            csString status;
            status.Format("SQL query %s, has taken %u time to process.\n", querystr.GetData(), timer.Stop());
            if(LogCSV::GetSingletonPtr())
                LogCSV::GetSingleton().Write(CSV_STATUS, status);
        }
        profs.AddSQLTime(querystr, timer.Stop());
        iResultSet *rs = new psResultSet(conn);
        return rs;
    }
    else
        return NULL;
}

int psMysqlConnection::SelectSingleNumber(const char *sql, ...)
{
    psStopWatch timer;
    csString querystr;
    va_list args;

    va_start(args, sql);
    querystr.FormatV(sql, args);
    va_end(args);

    lastquery = querystr;

    timer.Start();
    if (!mysql_query(conn, querystr))
    {
        if(timer.Stop() > 1000)
        {
            csString status;
            status.Format("SQL query %s, has taken %u time to process.\n", querystr.GetData(), timer.Stop());
            if(LogCSV::GetSingletonPtr())
                LogCSV::GetSingleton().Write(CSV_STATUS, status);
        }
        profs.AddSQLTime(querystr, timer.Stop());
        psResultSet *rs = new psResultSet(conn);

        if (rs->Count())
        {
            const char *value = (*rs)[0][0];
            int result = atoi( value?value:"0" );
            delete rs;
            return result;
        }
        else
        {
            delete rs;  // return err code below
        }
    }
    return QUERY_FAILED;
}

uint64 psMysqlConnection::GetLastInsertID()
{
    return mysql_insert_id(conn);
}

uint64 psMysqlConnection::GenericInsertWithID(const char *table,const char **fieldnames,psStringArray& fieldvalues)
{
    csString command;
    int count = fieldvalues.Length();
    int i;
    command = "INSERT INTO ";
    command.Append(table);
    command.Append(" (");
    for (i=0;i<count;i++)
    {
        if (i>0)
            command.Append(",");
        command.Append(fieldnames[i]);
    }

    command.Append(") VALUES (");
    for (i=0;i<count;i++)
    {
        if (i>0)
            command.Append(",");
        if (fieldvalues[i]!=NULL)
        {
            command.Append("'");
            csString escape;
            Escape( escape, fieldvalues[i] );
            command.Append(escape);
            command.Append("'");
        }
        else
        {
            command.Append("NULL");
        }
    }
    command.Append(")");

    if (Command(command)!=1)
        return 0;

    return GetLastInsertID();
}

bool psMysqlConnection::GenericUpdateWithID(const char *table,const char *idfield,const char *id,const char **fieldnames,psStringArray& fieldvalues)
{
    int i;
    int count = fieldvalues.Length();
    csString command;

    command.Append("UPDATE ");
    command.Append(table);
    command.Append(" SET ");

    for (i=0;i<count;i++)
    {
        if (i>0)
            command.Append(",");
        command.Append(fieldnames[i]);
        if (fieldvalues[i]!=NULL)
        {
            command.Append("='");
            csString escape;
            Escape(escape, fieldvalues[i]); 
            command.Append(escape);
            command.Append("'");
        }
        else
        {
            command.Append("=NULL");
        }

    }
   
    command.Append(" where ");
    command.Append(idfield);
    command.Append("='");
    csString escape;
    Escape( escape, id );
    command.Append(escape);
    command.Append("'");

    //printf("%s\n",command.GetData());

    if (CommandPump(command)==QUERY_FAILED)
    {
        return false;
    }

    return true;
}

const char *psMysqlConnection::uint64tostring(uint64 value,csString& recv)
{
    recv = "";

    while (value>0)
    {
        recv.Insert(0,(char)'0'+(value % 10) );
        value/=10;
    }
    return recv;
}

const char* psMysqlConnection::DumpProfile()
{
    profileDump = profs.Dump();
    return profileDump;
}

void psMysqlConnection::ResetProfile()
{
    profs.Reset();
    profileDump.Empty();
}

psResultSet::psResultSet(MYSQL *conn)
{
    rs = mysql_store_result(conn);
    if (rs)
    {
        rows    = (unsigned long) mysql_num_rows(rs);
        fields  = mysql_num_fields(rs);
        row.SetMaxFields(fields);
        row.SetResultSet((intptr_t)rs);

        current = (unsigned long) -1;
    }
    else
        rows = 0;
}

psResultSet::~psResultSet()
{
    mysql_free_result(rs);
}

iResultRow& psResultSet::operator[](unsigned long whichrow)
{
    if (whichrow != current)
    {
        if (!row.Fetch(whichrow))
            current = whichrow;
    }
    return row;
}

void psResultRow::SetResultSet(long resultsettoken)
{
    rs = (MYSQL_RES *)resultsettoken;
}

int psResultRow::Fetch(int row)
{
    mysql_data_seek(rs, row);
    
    rr = mysql_fetch_row(rs);

    if (rr)
        return 0;   // success
    else
    {
        max = 0;    // no fields will make operator[]'s safe
        return 1;
    }
}

const char *psResultRow::operator[](int whichfield)
{
    if (whichfield >= 0 && whichfield < max)
        return rr[whichfield];
    else
        return "";
}   

const char *psResultRow::operator[](const char *fieldname)
{
  MYSQL_FIELD *field;
  CS_ASSERT(fieldname);
  CS_ASSERT(max); // trying to access when no fields returned in row! probably empty resultset.

  int i;

  for ( i=last_index; (field=mysql_fetch_field(rs)); i++)
  {
    if (field->name && !strcasecmp(field->name,fieldname))
    {
      last_index = i+1;
      return rr[i];
    }
  }
  mysql_field_seek(rs, 0);
  for (i=0; (i<last_index) && (field=mysql_fetch_field(rs)); i++)
  {
    if (field->name && !strcasecmp(field->name,fieldname))
    {
      last_index = i+1;
      return rr[i];
    }
  }
  CPrintf(CON_BUG, "Could not find field %s!. Exiting.\n",fieldname);
  CS_ASSERT(false);
  return ""; // Illegal name.
}

int psResultRow::GetInt(int whichfield)
{
    const char *ptr = this->operator [](whichfield);
    return (ptr)?atoi(ptr):0;
}

int psResultRow::GetInt(const char *fieldname)
{
    const char *ptr = this->operator [](fieldname);
    return (ptr)?atoi(ptr):0;
}

unsigned long psResultRow::GetUInt32(int whichfield)
{
    const char *ptr = this->operator [](whichfield);
    return (ptr)?strtoul(ptr,NULL,10):0;
}

unsigned long psResultRow::GetUInt32(const char *fieldname)
{
    const char *ptr = this->operator [](fieldname);
    return (ptr)?strtoul(ptr,NULL,10):0;
}

float psResultRow::GetFloat(int whichfield)
{
    const char *ptr = this->operator [](whichfield);
    return (ptr)?atof(ptr):0;
}

float psResultRow::GetFloat(const char *fieldname)
{
    const char *ptr = this->operator [](fieldname);
    return (ptr)?atof(ptr):0;
}

uint64 psResultRow::GetUInt64(int whichfield)
{
    const char *ptr = this->operator [](whichfield);
    return (ptr)?stringtouint64(ptr):0;
}

uint64 psResultRow::GetUInt64(const char *fieldname)
{
    const char *ptr = this->operator [](fieldname);
    return (ptr)?stringtouint64(ptr):0;
}

uint64 psResultRow::stringtouint64(const char *stringbuf)
{
    /* Our goal is to handle a conversion from a string of base-10 digits into most any numeric format.
     *  This has very specific use and so we presume a few things here:
     *
     *  The value in the string can actually fit into the type being used
     *  This function can be slow compared to the smaller ato* and strto* counterparts.
     */
    uint64 result=0;
    CS_ASSERT(stringbuf!=NULL);
    while (*stringbuf!=0x00)
    {
        CS_ASSERT(*stringbuf >='0' && *stringbuf<='9');
        result=result * 10;
        result = result +  (uint64)(*stringbuf - (char)'0');
        stringbuf++;
    }

    return result;
}

#ifdef USE_DELAY_QUERY

DelayedQueryManager::DelayedQueryManager(const char *host, unsigned int port, const char *database,
                              const char *user, const char *pwd)
{
    start=end=0;
    arr.SetSize(THREADED_BUFFER_SIZE);
    m_Close = false;
    m_host = csString(host);
    m_port = port;
    m_db = csString(database);
    m_user = csString(user);
    m_pwd = csString(pwd);
}

void DelayedQueryManager::Stop()
{
    m_Close = true;
    datacondition.NotifyOne();
    CS::Threading::Thread::Yield();
}

void DelayedQueryManager::Run()
{
    mysql_thread_init();
    MYSQL *conn=mysql_init(NULL);
    m_conn = mysql_real_connect(conn,m_host,m_user,m_pwd,m_db,m_port,NULL,CLIENT_FOUND_ROWS);
    my_bool my_true = true;
    mysql_options(m_conn, MYSQL_OPT_RECONNECT, &my_true);    

    psStopWatch timer;
    while(!m_Close)
    {
        CS::Threading::MutexScopedLock lock(mutex);
        datacondition.Wait(mutex);
        while (start != end)
        {
            csString currQuery;
            {
                CS::Threading::RecursiveMutexScopedLock lock(mutexArray);
                currQuery = arr[end];
                end = (end+1) % arr.GetSize();
            }
            timer.Start();
            if (!mysql_query(m_conn, currQuery))
                profs.AddSQLTime(currQuery, timer.Stop());
        }
    }
    mysql_close(m_conn);
    mysql_thread_end();
}

void DelayedQueryManager::Push(csString query) 
{ 
    {
        CS::Threading::RecursiveMutexScopedLock lock(mutexArray);
        size_t tstart = (start+1) % arr.GetSize();
        if (tstart == end)
        {
            return;
        }
        arr[start] = query;
        start = tstart;
    }
    datacondition.NotifyOne();
}
#endif