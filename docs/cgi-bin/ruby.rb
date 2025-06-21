puts "Content-Type: text/plain\r\n"
puts "\r\n"

puts "Ruby CGI Script Information"
puts "========================\n"

puts "Ruby Version: #{RUBY_VERSION}"
puts "Server Time: #{Time.now}"
puts "Script Path: #{ENV['SCRIPT_FILENAME']}\n"