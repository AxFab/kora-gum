using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Reflection;

namespace GumSharp
{
    class Window : IDisposable
    {
        private static bool appInitialized = false;
        internal IntPtr Pointer { get; private set; }
        private Window(IntPtr ptr)
        {
            Pointer = ptr;
        }

        public static Window Open(int width, int height)
        {
            if (!appInitialized) {
                AppSetup_(Process.GetCurrentProcess().Handle);
                appInitialized = true;
            }
            IntPtr ptr = OpenWindow_(width, height);
            return ptr != IntPtr.Zero ? new Window(ptr) : null;
        }

        public void Dispose()
        {
            if (Pointer != IntPtr.Zero) {
                CloseWindow_(Pointer);
            }
            Pointer = IntPtr.Zero;
        }

        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_create_surface")]
        private extern static IntPtr OpenWindow_(int width, int height);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_destroy_surface")]
        private extern static IntPtr CloseWindow_(IntPtr win);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_win32_setup")]
        private extern static IntPtr AppSetup_(IntPtr hInstance);
    }
    class Cell
    {
        internal IntPtr Pointer { get; private set; }
        private Cell(IntPtr ptr)
        {
            Pointer = ptr;
        }

        public static Cell Load(string filename, SkinsPool skins)
        {
            IntPtr skinPtr = skins.Pointer;
            if (skinPtr == IntPtr.Zero)
                return null;
            IntPtr cellPtr = LoadXml_(filename, skinPtr);
            return cellPtr != IntPtr.Zero ? new Cell(cellPtr) : null;
        }

        public Cell GetById(string id)
        {
            IntPtr ptr = GetById_(Pointer, id);
            return GetSingle(ptr);
        }
   
        public void DestroyChildren()
        {
            DestroyChildren_(Pointer);
            // TODO - what about cache !?
            cellCache.Clear();
        }
        
        public void Append(Cell cell)
        {
            Append_(Pointer, cell.Pointer);
        }
        
        public Cell Clone()
        {
            IntPtr ptr = Clone_(Pointer);
            return GetSingle(ptr);
        }
        
        public Cell Remove()
        {
            Remove_(Pointer);
            return this;
        }
        
        static Dictionary<IntPtr, Cell> cellCache = new Dictionary<IntPtr, Cell>();
        
        private static Cell GetSingle(IntPtr ptr)
        {
            lock(cellCache) {
                Cell cell;
                if (!cell Cache.TryGetValue(ptr, out cell)) {
                    cell = new Cell(ptr);
                    cellCache.Add(ptr, cell);
                }
                return cell;
            }
        }
   
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_cell_loadxml")]
        private extern static IntPtr LoadXml_(string filename, IntPtr skins);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_get_by_id")]
        private extern static IntPtr GetById_(IntPtr cell, string id);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_cell_destroy_child")]
        private extern static void DestroyChildren_(IntPtr cell);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_cell_pushback")]
        private extern static void Append_(IntPtr cell, IntPtr child);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_cell_copy")]
        private extern static IntPtr Clone_(IntPtr cell);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_cell_detach")]
        private extern static void Remove_(IntPtr cell);
    }

    class SkinsPool
    {
        internal IntPtr Pointer { get; private set; }
        public SkinsPool()
        {
            Pointer = IntPtr.Zero;
        }

        public void LoadCss(string filename)
        {
            Pointer = LoadCss_(Pointer, filename);
        }
        
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_skins_loadcss")]
        private extern static IntPtr LoadCss_(IntPtr skins, string filename);
    }

    enum EventType
    {
        Unknow,
        Destroy,
        Expose,
        KeyPress,
        KeyUp,
        Motion,
        ButtonPress,
        ButtonOut,
        ButtonOver,
        ButtonDown,
        ButtonUp,
        ButtonFocus,
        ButtonLooseFocus,
        Click,
        DoubleClick,
        TripleClick,
        RightClick,
        
        Previous,
        Next,
        WheelUp,
        WheelDown,
        WheelClick,
        Resize
    }
    
    class Manager : IDisposable
    {
        internal IntPtr Pointer { get; private set; }
        private Manager(IntPtr ptr)
        {
            Pointer = ptr;
        }

        public static Manager Create(Cell root, Window win)
        {
            IntPtr ptr = CreateManager_(root.Pointer, win.Pointer);
            return ptr != IntPtr.Zero ? new Manager(ptr) : null;
        }

        public void Dispose()
        {
            if (Pointer != IntPtr.Zero) {
                // CloseManager_(Pointer);
            }
            Pointer = IntPtr.Zero;
        }

        public void Run()
        {
            EventLoop_(Pointer);
        }
        
        public void Bind(string id, EventType type, MethodInfo method)
        {
        }

        private delegate void InternEventHandler();
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_event_manager")]
        private extern static IntPtr CreateManager_(IntPtr root, IntPtr win);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_event_loop")]
        private extern static void EventLoop_(IntPtr manager);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_refresh")]
        private extern static void Refresh_(IntPtr manager);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_event_bind")]
        private extern static IntPtr BindEvent_(IntPtr manager, IntPtr cell, int type, InternEventHandler handler);    
    }

    class GumApp: IDisposable
    {
        public SkinsPool Skins { get; private set; }
        public Cell Root { get; private set; }
        public Window Window { get; private set; }
        public Manager Manager { get; private set; }
        
        protected GumApp()
        {
            string name = "browser/app2";
            Skins = new SkinsPool();
            Skins.LoadCss("./resx/" + name + ".css");
            Root = Cell.Load("./resx/" + name + ".xml", Skins);
            int width = 680; // TODO - Read from root
            int height = 425;
            Window = Window.Open(width, height);
            Manager = Manager.Create(Root, Window);
            
            foreach (var method in GetType().GetMethods()) {
                BindEventAttribute[] attributes = (BindEventAttribute[])method.GetCustomAttributes(typeof(BindEventAttribute), false);
                foreach (var attribute in attributes) {
                    Manager.Bind(attribute.Id, attrinute.Type, method);
                }
            }
        }
        
        public void Dispose()
        {
            if (Manager != null)
                Manager.Dispose();
            if (Window != null)
                Window.Dispose();
        }
    }
    
    class BindEventAttribute: Attribute
    {
        public BindEventAttribute(EventType type): this(null, type)
        {
        }
        
        public BindEventAttribute(string id, EventType type)
        {
            Id = id;
            Type = type;
        }
        
        public string Id { get; private set; }
        public EventType Type { get; private set; }
    }

    class Browser: GumApp
    {
        FileInfo directory;
        Cell view;
        Cell icon;
        Cell ctxFile;
        Cell ctxView;
        
        public Browser(): this(Environment.CurrentDirectory)
        {
        }
        
        public Browser(string dir)
        {
            directory = new FileInfo(dir);
            view = Root.GetById("view");
            icon = Root.GetById("icon");
            ctxFile = Root.GetById("ctx-menu-file").Remove();
            ctxView = Root.GetById("ctx-menu-view").Remove();
            OnRefresh(null);
        }
        
        static void Main(string[] args)
        {
            using (GumApp app = new Browser())
                app.Manager.Run();
        }
        
        //[BindEvent(EventType.Previous)]
        [BindEvent("btn-prev", EventType.Click)]
        public void OnPrevious(Cell target)
        {
        }
        
        //[BindEvent(EventType.Next)]
        [BindEvent("btn-next", EventType.Click)]
        public void OnNext(Cell target)
        {
        }
        
        [BindEvent("btn-top", EventType.Click)]
        public void OnParent(Cell target)
        {
            directory = new FileInfo(directory.DirectoryName);
            OnRefresh(target);
        }
        
        [BindEvent("btn-refr", EventType.Click)]
        public void OnRefresh(Cell target)
        {
            while(directory != null && (!directory.Exists || !directory.Attributes.HasFlag(FileAttributes.Directory)))
                directory = new FileInfo(directory.DirectoryName);
            if (directory == null)
                directory = new FileInfo("C:/");
            foreach (string file in Directory.EnumerateFileSystemEntries(directory.Fullname)) {
                // icon.SetTextAndIcon();
                // view.Append(icon.Clone());
            }
        }
    }
}
