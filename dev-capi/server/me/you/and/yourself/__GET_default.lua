local json = require("json")

-- Function to check if a string is JSON
local function isJSON(str)
    local success, result = pcall(json.decode, str)
    return success and type(result) == "table"
end

function test()
    local query = "SELECT * FROM table_1;"
    local result = query_select_Lua(query)
    local response
    if result ~= "FAILED" then 
        if isJSON(result) then
            response = format_response_Lua(200, "Content-Type: application/json", result)
        else
            response = format_response_Lua(200, "Content-Type: text/plain", result)
        end
    else 
        response = format_response_Lua(400, "Content-Type: text/html", "<html><body><h1>400 Bad Request</h1><p>Your request is invalid.</p></body></html>")
    end
    return response
end
