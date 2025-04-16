#include "SQLManager.h"


#include "bcrypt.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

void SQLManager::CheckUsersTable()
{
    try
    {
        // Try to select
        std::string query = "SELECT 1 FROM Users LIMIT 1";
        sql::PreparedStatement* statement = connection->prepareStatement(query);

        statement->executeQuery();

        delete statement;
        return;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "Table not found, creating table: " << e.what() << std::endl;

        // Creación de tabla si no existe
        try
        {
            std::string createQuery = "CREATE TABLE Users ("
                "id INT AUTO_INCREMENT PRIMARY KEY, "
                "username VARCHAR(50), "
                "password VARCHAR(250))";
            sql::PreparedStatement* createStatement = connection->prepareStatement(createQuery);
            createStatement->execute();
            delete createStatement;
            std::cout << "Table 'Usersa' created successfully." << std::endl;
        }
        catch (sql::SQLException& createError)
        {
            std::cerr << "Error creating table: " << createError.what() << std::endl;
        }

        return;
    }
}

void SQLManager::ConnectDatabase()
{
    try
    {
        driver = get_driver_instance();
        connection = driver->connect(SQL_IP, SQL_USER, SQL_PASSWORD);
        connection->setSchema(SQL_DATABASE);
        std::cout << "Connection Done!" << std::endl;

        CheckUsersTable();

    }
    catch (sql::SQLException e)
    {
        std::cerr << "Could not connect to server. Error message: " << e.what() << std::endl;
    }
}

bool SQLManager::InsertUser(std::string username, std::string password)
{
    std::string hash = bcrypt::generateHash(password);
    try
    {
        // Query on \SQL\Database Creation Scripts.sql
        std::string query = "INSERT INTO Users (username, password)"
            "SELECT * FROM(SELECT ? AS username, ? AS password) AS TemporalTable "
            "WHERE NOT EXISTS (SELECT id FROM Users WHERE username = ?)";
        sql::PreparedStatement* statement = connection->prepareStatement(query);

        statement->setString(1, username);
        statement->setString(2, hash);
        statement->setString(3, username);

        int affected_rows = statement->executeUpdate();

        delete statement;

        if (affected_rows > 0)
        {
            std::cout << "User Inserted Successfully" << std::endl;
            return true;
        }
        std::cerr << "User Not Inserted" << std::endl;

        return false;

    }
    catch (sql::SQLException& e)
    {
        std::cerr << "Error while inserting user: " << e.what() << std::endl;
        return false;
    }
}

int SQLManager::CheckLogin(std::string username, std::string password)
{
    try
    {
        // Query on \SQL\Database Creation Scripts.sql
        std::string query = "SELECT id, password FROM Users WHERE username = ?";
        sql::PreparedStatement* statement = connection->prepareStatement(query);
        statement->setString(1, username);

        // We obtain all the values returned from the Query
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery());

        delete statement;

        if (result->next())
        {
            std::string storedHash = result->getString("password");

            // Validate hast using bcrypt::validatePassword

            if (bcrypt::validatePassword(password, storedHash))
            {
                int userID = result->getInt("id");
                std::cout << "User exists with ID: " << userID << std::endl;
                return userID;
            }
            else
            {
                std::cerr << "Invalid password" << std::endl;
                return -1;
            }
        }
        std::cerr << "Username not found" << std::endl;
        return -1;

    }
    catch (sql::SQLException& e)
    {
        std::cerr << "Error while getting user: " << e.what() << std::endl;
        return -1;
    }
}

void SQLManager::DisconnectDatabase()
{
    connection->close();

    if (connection->isClosed())
    {
        std::cout << "Connection Close" << std::endl;
    }
}
