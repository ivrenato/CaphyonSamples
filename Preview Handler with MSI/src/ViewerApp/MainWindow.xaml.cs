using System.IO;
using System.Windows;
using Microsoft.Win32;

namespace ViewerApp
{
    public partial class MainWindow : Window
    {
        public MainWindow(string? filePath)
        {
            InitializeComponent();

            if (!string.IsNullOrEmpty(filePath))
                OpenFile(filePath);
        }

        private void OpenFile(string path)
        {
            try
            {
               
                ContentBox.Text = File.ReadAllText(path);
                Title = $"ViewerApp - {Path.GetFileName(path)}";
            }
            catch (Exception ex)
            {
                ContentBox.Text = $"Could not open file:\n{path}\n\n{ex.Message}";
            }
        }

        private void OpenButton_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog
            {
                Filter = "XYZ files (*.xyz)|*.xyz|All files (*.*)|*.*"
            };

            if (dialog.ShowDialog(this) == true)
                OpenFile(dialog.FileName);
        }

        private void CloseButton_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
