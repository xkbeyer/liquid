def test_var()
    var id = 5
    int x = 24 + id
    display("id = %d x = %d\n", id, x)
    var otto = "Hello"
    displayln("otto = %s (Hello)", otto)
    otto = "It's me."
    displayln("otto = %s (It's me.)", otto)
    otto = 5 # This should result in an error message. (No change to the value of otto).
    displayln("otto = %s (It's me.)", otto)


