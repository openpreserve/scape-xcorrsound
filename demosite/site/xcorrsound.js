function resetForm() {
    document.getElementById('compare_form').reset();
    resetResults();
}

function resetResults() {
    document.getElementById("content").innerHTML = "";
    document.getElementById("progressBar").className = "";
    document.getElementById("progressBar").innerHTML = "";
}

/**Ajax Request (Submits the form below through AJAX
 *               and then calls the ajax_response function)
 */
function ajax_request(xcorrsoundFunction) {
    document.getElementById("progressBar").className = "spinner";
    document.getElementById("content").innerHTML = "Processing...";

    var submitTo = '../xcorrsound.php';
    var formData = new FormData();
    formData.append('xcorrsoundFunction', xcorrsoundFunction);
    formData.append('file_select_1', document.getElementById('file_select_1').files[0]);
    formData.append('file_select_2', document.getElementById('file_select_2').files[0]);
    formData.append('demo_select_1', document.getElementById('demo_select_1').value);
    formData.append('demo_select_2', document.getElementById('demo_select_2').value);

    http_uploadBinaries('POST', submitTo, ajax_response, formData);
}

/**Ajax Response (Called when ajax data has been retrieved)
 *
 * @param   object  data   Javascript (JSON) data object received
 *                         through ajax call
 */
function ajax_response(data) {
    resetResults();
    document.getElementById("content").innerHTML = data;
}

function reset_demo_select_1() {
    document.getElementById("demo_select_1").value = "";
    document.getElementById("demo_select_1").selectedIndex = "0";
// 		printSelectedItems("reset_demo_select_1");
}
function reset_demo_select_2() {
    document.getElementById("demo_select_2").value = "";
    document.getElementById("demo_select_2").selectedIndex = "0";
// 		printSelectedItems("reset_demo_select_2");
}

function reset_file_select_1() {
    document.getElementById("file_select_1").value = "";
// 		printSelectedItems("reset_file_select_1");
}
function reset_file_select_2() {
    document.getElementById("file_select_2").value = "";
// 		printSelectedItems("reset_file_select_2");
}

function printSelectedItems(invokedFunction) {
    var selectedItems = "demo1: " + document.getElementById("demo_select_1").value + "<br />"
        + "demo2: " + document.getElementById("demo_select_2").value + "<br />"
        + "file1: " + document.getElementById("file_select_1").value + "<br />"
        + "file2: " + document.getElementById("file_select_2").value;

    document.getElementById("content").innerHTML = invokedFunction + " invoked <br />" + selectedItems;
}

